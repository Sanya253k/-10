#!/usr/bin/env python3

import ctypes as _ctypes

from pathlib import Path

import sys as _sys


# Загрузка библиотеки

_CURRENT_DIR = Path(__file__).resolve().parent

_LIB_PATHS = [
    _CURRENT_DIR / ".." / "build" / "libpolynomial.so",
    _CURRENT_DIR / "libpolynomial.so",
]


_lib = None
for _path in _LIB_PATHS:
    if _path.exists():
        _lib = _ctypes.CDLL(str(_path))
        break


if _lib is None:
    print("Ошибка: не удалось найти libpolynomial.so", file=_sys.stderr)
    print("Выполните: make shared", file=_sys.stderr)
    _sys.exit(1)


# Коды ошибок

POLY_OK = 0
POLY_ERROR_NULL = 1
POLY_ERROR_MEMORY = 2
POLY_ERROR_SIZE = 3


# Структура polynomial_t

class PolynomialT(_ctypes.Structure):
    """Отображение структуры polynomial_t из C."""

    _fields_ = [
        ("coeffs", _ctypes.POINTER(_ctypes.c_double)),
        ("degree", _ctypes.c_size_t),
    ]


PolynomialPtr = _ctypes.POINTER(PolynomialT)


# Прототипы функций

_lib.poly_create.argtypes = [PolynomialPtr, _ctypes.c_size_t]
_lib.poly_create.restype = _ctypes.c_int

_lib.poly_free.argtypes = [PolynomialPtr]
_lib.poly_free.restype = None

_lib.poly_evaluate.argtypes = [
    PolynomialPtr,
    _ctypes.c_double,
    _ctypes.POINTER(_ctypes.c_double),
]
_lib.poly_evaluate.restype = _ctypes.c_int

_lib.poly_add.argtypes = [PolynomialPtr, PolynomialPtr, PolynomialPtr]
_lib.poly_add.restype = _ctypes.c_int

_lib.poly_sub.argtypes = [PolynomialPtr, PolynomialPtr, PolynomialPtr]
_lib.poly_sub.restype = _ctypes.c_int

_lib.poly_scale.argtypes = [PolynomialPtr, _ctypes.c_double]
_lib.poly_scale.restype = _ctypes.c_int

_lib.poly_derivative.argtypes = [PolynomialPtr, PolynomialPtr]
_lib.poly_derivative.restype = _ctypes.c_int


# Класс накопления результатов


class TestState:
    """Простой счётчик прошедших и упавших тестов."""

    def __init__(self):
        self.passed = 0
        self.failed = 0

    def check(self, condition: bool, message: str) -> None:
        if condition:
            print(f"[PASS] {message}")
            self.passed += 1
        else:
            print(f"[FAIL] {message}")
            self.failed += 1


# Вспомогательные функции

EPS = 1e-9


def make_poly(degree: int, coeffs: list | None = None) -> PolynomialT:
    """Создать polynomial_t заданной степени и опционально задать коэффициенты."""
    p = PolynomialT()
    err = _lib.poly_create(_ctypes.byref(p), _ctypes.c_size_t(degree))
    assert err == POLY_OK, f"poly_create вернул {err}"
    if coeffs is not None:
        for i, v in enumerate(coeffs):
            p.coeffs[i] = v
    return p


def free_poly(p: PolynomialT) -> None:
    """Освободить polynomial_t."""
    _lib.poly_free(_ctypes.byref(p))


def get_coeffs(p: PolynomialT) -> list:
    """Вернуть список коэффициентов многочлена."""
    return [p.coeffs[i] for i in range(p.degree + 1)]


# Тестовые группы


def test_create_free(ts: TestState) -> None:
    print("\n[Группа] poly_create / poly_free")

    # Типовой: многочлен степени 3
    p = PolynomialT()
    err = _lib.poly_create(_ctypes.byref(p), _ctypes.c_size_t(3))
    ts.check(err == POLY_OK, "create степень 3: код OK")
    ts.check(p.coeffs, "create степень 3: coeffs не NULL")
    ts.check(p.degree == 3, "create степень 3: degree == 3")
    ts.check(
        abs(p.coeffs[0]) < EPS and abs(p.coeffs[3]) < EPS,
        "create степень 3: коэффициенты обнулены",
    )
    _lib.poly_free(_ctypes.byref(p))
    ts.check(not p.coeffs, "free: coeffs == NULL после free")
    ts.check(p.degree == 0, "free: degree == 0 после free")

    # Граничный: многочлен степени 0 (константа)
    p = PolynomialT()
    err = _lib.poly_create(_ctypes.byref(p), _ctypes.c_size_t(0))
    ts.check(err == POLY_OK, "create степень 0: код OK")
    ts.check(p.degree == 0, "create степень 0: degree == 0")
    _lib.poly_free(_ctypes.byref(p))

    # Ошибочный: NULL-указатель
    err = _lib.poly_create(None, _ctypes.c_size_t(3))
    ts.check(err == POLY_ERROR_NULL, "create NULL -> POLY_ERROR_NULL")

    # Безопасный вызов free с NULL-указателем
    _lib.poly_free(None)
    ts.check(True, "free(NULL): безопасен")


def test_evaluate(ts: TestState) -> None:
    print("\n[Группа] poly_evaluate")

    # Типовой: p(x) = 1 + 2x + 3x^2, x = 2 → 1+4+12 = 17
    p = make_poly(2, [1.0, 2.0, 3.0])
    result = _ctypes.c_double(0.0)
    err = _lib.poly_evaluate(_ctypes.byref(p), _ctypes.c_double(2.0), _ctypes.byref(result))
    ts.check(err == POLY_OK, "evaluate типовой: код OK")
    ts.check(abs(result.value - 17.0) < EPS, "evaluate типовой: p(2)==17")
    free_poly(p)

    # Граничный: константный многочлен p(x) = 5, x = 100 → 5
    p = make_poly(0, [5.0])
    result = _ctypes.c_double(0.0)
    _lib.poly_evaluate(_ctypes.byref(p), _ctypes.c_double(100.0), _ctypes.byref(result))
    ts.check(abs(result.value - 5.0) < EPS, "evaluate константа: p(100)==5")
    free_poly(p)

    # Граничный: нулевой многочлен, x = 999 → 0
    p = make_poly(4)
    result = _ctypes.c_double(0.0)
    _lib.poly_evaluate(_ctypes.byref(p), _ctypes.c_double(999.0), _ctypes.byref(result))
    ts.check(abs(result.value) < EPS, "evaluate нулевой многочлен: p(x)==0")
    free_poly(p)

    # Граничный: x = 0 → только свободный член
    p = make_poly(2, [7.0, 99.0, 99.0])
    result = _ctypes.c_double(0.0)
    _lib.poly_evaluate(_ctypes.byref(p), _ctypes.c_double(0.0), _ctypes.byref(result))
    ts.check(abs(result.value - 7.0) < EPS, "evaluate x=0: только coeffs[0]")
    free_poly(p)

    # Ошибочный: NULL-указатель на многочлен
    result = _ctypes.c_double(0.0)
    err = _lib.poly_evaluate(None, _ctypes.c_double(1.0), _ctypes.byref(result))
    ts.check(err == POLY_ERROR_NULL, "evaluate NULL poly -> POLY_ERROR_NULL")

    # Ошибочный: NULL-указатель на результат
    p = make_poly(1)
    err = _lib.poly_evaluate(_ctypes.byref(p), _ctypes.c_double(1.0), None)
    ts.check(err == POLY_ERROR_NULL, "evaluate NULL result -> POLY_ERROR_NULL")
    free_poly(p)


def test_add(ts: TestState) -> None:
    print("\n[Группа] poly_add")

    # Типовой: одинаковая степень, (1+2x+3x^2) + (1+2x+3x^2) = 2+4x+6x^2
    a = make_poly(2, [1.0, 2.0, 3.0])
    b = make_poly(2, [1.0, 2.0, 3.0])
    res = PolynomialT()
    err = _lib.poly_add(_ctypes.byref(a), _ctypes.byref(b), _ctypes.byref(res))
    ts.check(err == POLY_OK, "add одинак. степень: OK")
    ts.check(res.degree == 2, "add одинак. степень: degree")
    cs = get_coeffs(res)
    ts.check(
        abs(cs[0] - 2.0) < EPS and abs(cs[1] - 4.0) < EPS and abs(cs[2] - 6.0) < EPS,
        "add одинак. степень: коэффициенты",
    )
    free_poly(a); free_poly(b); free_poly(res)

    # Граничный: разные степени — результат имеет степень большего
    a = make_poly(1, [1.0, 1.0])
    b = make_poly(3, [0.0, 0.0, 0.0, 4.0])
    res = PolynomialT()
    _lib.poly_add(_ctypes.byref(a), _ctypes.byref(b), _ctypes.byref(res))
    ts.check(res.degree == 3, "add разные степени: degree==3")
    ts.check(abs(res.coeffs[3] - 4.0) < EPS, "add разные степени: coeffs[3]")
    free_poly(a); free_poly(b); free_poly(res)

    # Граничный: сложение с нулевым многочленом
    a = make_poly(2, [3.0, 1.0, 2.0])
    b = make_poly(2)
    res = PolynomialT()
    _lib.poly_add(_ctypes.byref(a), _ctypes.byref(b), _ctypes.byref(res))
    cs = get_coeffs(res)
    ts.check(
        abs(cs[0] - 3.0) < EPS and abs(cs[1] - 1.0) < EPS and abs(cs[2] - 2.0) < EPS,
        "add с нулевым: результат совпадает с a",
    )
    free_poly(a); free_poly(b); free_poly(res)

    # Ошибочный: NULL
    err = _lib.poly_add(None, None, None)
    ts.check(err == POLY_ERROR_NULL, "add NULL -> POLY_ERROR_NULL")


def test_sub(ts: TestState) -> None:
    print("\n[Группа] poly_sub")

    # Типовой: (3+4x) - (1+2x) = 2+2x
    a = make_poly(1, [3.0, 4.0])
    b = make_poly(1, [1.0, 2.0])
    res = PolynomialT()
    err = _lib.poly_sub(_ctypes.byref(a), _ctypes.byref(b), _ctypes.byref(res))
    ts.check(err == POLY_OK, "sub типовой: OK")
    cs = get_coeffs(res)
    ts.check(abs(cs[0] - 2.0) < EPS and abs(cs[1] - 2.0) < EPS, "sub типовой: коэффициенты")
    free_poly(a); free_poly(b); free_poly(res)

    # Граничный: вычитание из самого себя → нулевой
    a = make_poly(2, [5.0, 3.0, 1.0])
    res = PolynomialT()
    _lib.poly_sub(_ctypes.byref(a), _ctypes.byref(a), _ctypes.byref(res))
    cs = get_coeffs(res)
    ts.check(
        all(abs(c) < EPS for c in cs),
        "sub a-a: все коэффициенты нулевые",
    )
    free_poly(a); free_poly(res)

    # Граничный: разные степени
    a = make_poly(3, [1.0, 1.0, 0.0, 1.0])
    b = make_poly(1, [1.0, 1.0])
    res = PolynomialT()
    _lib.poly_sub(_ctypes.byref(a), _ctypes.byref(b), _ctypes.byref(res))
    ts.check(res.degree == 3, "sub разные степени: degree")
    cs = get_coeffs(res)
    ts.check(
        abs(cs[0]) < EPS and abs(cs[1]) < EPS and abs(cs[3] - 1.0) < EPS,
        "sub разные степени: коэффициенты",
    )
    free_poly(a); free_poly(b); free_poly(res)

    # Ошибочный: NULL
    err = _lib.poly_sub(None, None, None)
    ts.check(err == POLY_ERROR_NULL, "sub NULL -> POLY_ERROR_NULL")


def test_scale(ts: TestState) -> None:
    print("\n[Группа] poly_scale")

    # Типовой: (1+2x+3x^2) * 2 = 2+4x+6x^2
    p = make_poly(2, [1.0, 2.0, 3.0])
    err = _lib.poly_scale(_ctypes.byref(p), _ctypes.c_double(2.0))
    ts.check(err == POLY_OK, "scale типовой: OK")
    cs = get_coeffs(p)
    ts.check(
        abs(cs[0] - 2.0) < EPS and abs(cs[1] - 4.0) < EPS and abs(cs[2] - 6.0) < EPS,
        "scale типовой: коэффициенты",
    )
    free_poly(p)

    # Граничный: умножение на 0 → нулевой многочлен
    p = make_poly(2, [9.0, 9.0, 9.0])
    _lib.poly_scale(_ctypes.byref(p), _ctypes.c_double(0.0))
    cs = get_coeffs(p)
    ts.check(all(abs(c) < EPS for c in cs), "scale на 0: все коэффициенты нулевые")
    free_poly(p)

    # Граничный: умножение на -1
    p = make_poly(1, [3.0, -5.0])
    _lib.poly_scale(_ctypes.byref(p), _ctypes.c_double(-1.0))
    cs = get_coeffs(p)
    ts.check(abs(cs[0] + 3.0) < EPS and abs(cs[1] - 5.0) < EPS, "scale на -1: знаки инвертированы")
    free_poly(p)

    # Ошибочный: NULL
    err = _lib.poly_scale(None, _ctypes.c_double(2.0))
    ts.check(err == POLY_ERROR_NULL, "scale NULL -> POLY_ERROR_NULL")


def test_derivative(ts: TestState) -> None:
    print("\n[Группа] poly_derivative")

    # Типовой: d/dx(1 + 2x + 3x^2) = 2 + 6x
    p = make_poly(2, [1.0, 2.0, 3.0])
    res = PolynomialT()
    err = _lib.poly_derivative(_ctypes.byref(p), _ctypes.byref(res))
    ts.check(err == POLY_OK, "derivative типовой: OK")
    ts.check(res.degree == 1, "derivative типовой: degree==1")
    cs = get_coeffs(res)
    ts.check(abs(cs[0] - 2.0) < EPS and abs(cs[1] - 6.0) < EPS, "derivative типовой: коэффициенты")
    free_poly(p); free_poly(res)

    # Граничный: d/dx(константа) = 0
    p = make_poly(0, [42.0])
    res = PolynomialT()
    err = _lib.poly_derivative(_ctypes.byref(p), _ctypes.byref(res))
    ts.check(err == POLY_OK, "derivative константа: OK")
    ts.check(res.degree == 0, "derivative константа: degree==0")
    ts.check(abs(res.coeffs[0]) < EPS, "derivative константа: coeffs==0")
    free_poly(p); free_poly(res)

    # Граничный: d/dx(x^3) = 3x^2
    p = make_poly(3)
    p.coeffs[3] = 1.0
    res = PolynomialT()
    _lib.poly_derivative(_ctypes.byref(p), _ctypes.byref(res))
    ts.check(res.degree == 2, "derivative x^3: degree==2")
    cs = get_coeffs(res)
    ts.check(
        abs(cs[0]) < EPS and abs(cs[1]) < EPS and abs(cs[2] - 3.0) < EPS,
        "derivative x^3: coeffs[2]==3",
    )
    free_poly(p); free_poly(res)

    # Ошибочный: NULL poly
    res = PolynomialT()
    err = _lib.poly_derivative(None, _ctypes.byref(res))
    ts.check(err == POLY_ERROR_NULL, "derivative NULL poly -> POLY_ERROR_NULL")

    # Ошибочный: NULL result
    p = make_poly(2)
    err = _lib.poly_derivative(_ctypes.byref(p), None)
    ts.check(err == POLY_ERROR_NULL, "derivative NULL result -> POLY_ERROR_NULL")
    free_poly(p)


def test_large_polynomial(ts: TestState) -> None:
    """Проверить работу на многочлене большой степени."""
    print("\n[Группа] Многочлен большой степени")

    degree = 1000

    # Создать многочлен: coeffs[i] = i
    p = make_poly(degree)
    for i in range(degree + 1):
        p.coeffs[i] = float(i)

    # Вычислить p(1) = 0+1+2+...+1000 = 500500
    result = _ctypes.c_double(0.0)
    err = _lib.poly_evaluate(_ctypes.byref(p), _ctypes.c_double(1.0), _ctypes.byref(result))
    expected = sum(range(degree + 1))
    ts.check(err == POLY_OK, "большой: evaluate OK")
    ts.check(abs(result.value - expected) < 1e-3, f"большой: p(1)=={expected}")

    # Сложить с самим собой: каждый коэффициент удваивается
    res_add = PolynomialT()
    err = _lib.poly_add(_ctypes.byref(p), _ctypes.byref(p), _ctypes.byref(res_add))
    ts.check(err == POLY_OK, "большой: add OK")
    ts.check(res_add.degree == degree, "большой: add degree совпадает")
    ts.check(
        abs(res_add.coeffs[degree] - 2.0 * degree) < EPS,
        "большой: add старший коэффициент удвоен",
    )
    free_poly(res_add)

    # Производная: degree снизится на 1
    res_der = PolynomialT()
    err = _lib.poly_derivative(_ctypes.byref(p), _ctypes.byref(res_der))
    ts.check(err == POLY_OK, "большой: derivative OK")
    ts.check(res_der.degree == degree - 1, "большой: derivative degree == degree-1")
    # coeffs[i] исходного = i, после дифф coeffs[i-1] = i*i
    ts.check(
        abs(res_der.coeffs[degree - 1] - float(degree * degree)) < EPS,
        "большой: derivative старший коэффициент верен",
    )
    free_poly(res_der)

    free_poly(p)


# Точка входа

if __name__ == "__main__":
    ts = TestState()

    test_create_free(ts)
    test_evaluate(ts)
    test_add(ts)
    test_sub(ts)
    test_scale(ts)
    test_derivative(ts)
    test_large_polynomial(ts)

    print(f"\nИтого: {ts.passed} прошло, {ts.failed} упало")

    _sys.exit(0 if ts.failed == 0 else 1)
