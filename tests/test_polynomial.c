#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/polynomial.h"

/** Погрешность для сравнения вещественных чисел. */
#define TEST_EPS 1e-9

/** Структура для накопления счётчиков прошедших/упавших тестов. */
typedef struct {
    size_t passed; /**< Число прошедших тестов. */
    size_t failed; /**< Число упавших тестов.   */
} test_state_t;

/**
 * Проверить условие и обновить счётчики.
 */
#define TEST_CHECK(expr, msg, ptr_ts)     \
    do {                                  \
        if (expr) {                       \
            printf("[PASS] %s\n", (msg)); \
            (ptr_ts)->passed++;           \
        } else {                          \
            printf("[FAIL] %s\n", (msg)); \
            (ptr_ts)->failed++;           \
        }                                 \
    } while (0)


static void test_create_free(test_state_t *ptr_ts)
{
    polynomial_t p;
    poly_status_t status;

    printf("\n[Группа] poly_create / poly_free\n");

    /* Типовой случай: многочлен степени 3 */
    {
        status = poly_create(&p, 3);
        TEST_CHECK(status == POLY_OK,   "create степень 3: код OK",           ptr_ts);
        TEST_CHECK(p.coeffs != NULL,    "create степень 3: coeffs не NULL",   ptr_ts);
        TEST_CHECK(p.degree == 3,       "create степень 3: degree == 3",      ptr_ts);
        /* calloc должен обнулить коэффициенты */
        TEST_CHECK(fabs(p.coeffs[0]) < TEST_EPS &&
                   fabs(p.coeffs[3]) < TEST_EPS,
                   "create степень 3: коэффициенты обнулены",                 ptr_ts);
        poly_free(&p);
        TEST_CHECK(p.coeffs == NULL,    "free: coeffs == NULL после free",    ptr_ts);
        TEST_CHECK(p.degree == 0,       "free: degree == 0 после free",       ptr_ts);
    }

    /* Граничный случай: многочлен степени 0 (константа) */
    {
        status = poly_create(&p, 0);
        TEST_CHECK(status == POLY_OK,   "create степень 0: код OK",           ptr_ts);
        TEST_CHECK(p.degree == 0,       "create степень 0: degree == 0",      ptr_ts);
        poly_free(&p);
    }

    /* Ошибочный случай: NULL-указатель */
    {
        status = poly_create(NULL, 3);
        TEST_CHECK(status == POLY_ERROR_NULL, "create NULL -> POLY_ERROR_NULL", ptr_ts);
    }

    /* Безопасный вызов free с NULL-указателем */
    {
        poly_free(NULL);
        TEST_CHECK(1, "free(NULL): безопасен", ptr_ts);
    }
}


static void test_evaluate(test_state_t *ptr_ts)
{
    polynomial_t p;
    double       result = 0.0;
    poly_status_t status;

    printf("\n[Группа] poly_evaluate\n");

    /* Типовой случай: p(x) = 1 + 2x + 3x^2, x = 2 → 1+4+12 = 17 */
    {
        poly_create(&p, 2);
        p.coeffs[0] = 1.0;
        p.coeffs[1] = 2.0;
        p.coeffs[2] = 3.0;
        status = poly_evaluate(&p, 2.0, &result);
        TEST_CHECK(status == POLY_OK,              "evaluate типовой: код OK",   ptr_ts);
        TEST_CHECK(fabs(result - 17.0) < TEST_EPS, "evaluate типовой: p(2)==17", ptr_ts);
        poly_free(&p);
    }

    /* Граничный случай: константный многочлен p(x) = 5, x = 100 → 5 */
    {
        poly_create(&p, 0);
        p.coeffs[0] = 5.0;
        status = poly_evaluate(&p, 100.0, &result);
        TEST_CHECK(fabs(result - 5.0) < TEST_EPS,  "evaluate константа: p(100)==5", ptr_ts);
        poly_free(&p);
    }

    /* Граничный случай: нулевой многочлен, x = 999 → 0 */
    {
        poly_create(&p, 4);
        status = poly_evaluate(&p, 999.0, &result);
        TEST_CHECK(fabs(result) < TEST_EPS, "evaluate нулевой многочлен: p(x)==0", ptr_ts);
        poly_free(&p);
    }

    /* Граничный случай: x = 0 → только свободный член */
    {
        poly_create(&p, 2);
        p.coeffs[0] = 7.0;
        p.coeffs[1] = 99.0;
        p.coeffs[2] = 99.0;
        status = poly_evaluate(&p, 0.0, &result);
        TEST_CHECK(fabs(result - 7.0) < TEST_EPS, "evaluate x=0: только coeffs[0]", ptr_ts);
        poly_free(&p);
    }

    /* Ошибочный случай: NULL-указатель на многочлен */
    {
        status = poly_evaluate(NULL, 1.0, &result);
        TEST_CHECK(status == POLY_ERROR_NULL, "evaluate NULL poly -> POLY_ERROR_NULL", ptr_ts);
    }

    /* Ошибочный случай: NULL-указатель на результат */
    {
        poly_create(&p, 1);
        status = poly_evaluate(&p, 1.0, NULL);
        TEST_CHECK(status == POLY_ERROR_NULL, "evaluate NULL result -> POLY_ERROR_NULL", ptr_ts);
        poly_free(&p);
    }
}


static void test_add(test_state_t *ptr_ts)
{
    polynomial_t a, b, res;
    poly_status_t status;

    printf("\n[Группа] poly_add\n");

    /* Типовой случай: одинаковая степень, (1+2x+3x^2) + (1+2x+3x^2) = 2+4x+6x^2 */
    {
        poly_create(&a, 2);
        poly_create(&b, 2);
        a.coeffs[0] = 1.0; a.coeffs[1] = 2.0; a.coeffs[2] = 3.0;
        b.coeffs[0] = 1.0; b.coeffs[1] = 2.0; b.coeffs[2] = 3.0;
        status = poly_add(&a, &b, &res);
        TEST_CHECK(status == POLY_OK,                    "add одинак. степень: OK",     ptr_ts);
        TEST_CHECK(res.degree == 2,                      "add одинак. степень: degree", ptr_ts);
        TEST_CHECK(fabs(res.coeffs[0] - 2.0) < TEST_EPS &&
                   fabs(res.coeffs[1] - 4.0) < TEST_EPS &&
                   fabs(res.coeffs[2] - 6.0) < TEST_EPS,
                   "add одинак. степень: коэффициенты",                                ptr_ts);
        poly_free(&a); poly_free(&b); poly_free(&res);
    }

    /* Граничный случай: разные степени — результат имеет степень большего */
    {
        poly_create(&a, 1);
        poly_create(&b, 3);
        a.coeffs[0] = 1.0; a.coeffs[1] = 1.0;
        b.coeffs[0] = 0.0; b.coeffs[1] = 0.0; b.coeffs[2] = 0.0; b.coeffs[3] = 4.0;
        status = poly_add(&a, &b, &res);
        TEST_CHECK(res.degree == 3,                      "add разные степени: degree==3", ptr_ts);
        TEST_CHECK(fabs(res.coeffs[3] - 4.0) < TEST_EPS,"add разные степени: coeffs[3]", ptr_ts);
        poly_free(&a); poly_free(&b); poly_free(&res);
    }

    /* Граничный случай: сложение с нулевым многочленом */
    {
        poly_create(&a, 2);
        poly_create(&b, 2);
        a.coeffs[0] = 3.0; a.coeffs[1] = 1.0; a.coeffs[2] = 2.0;
        status = poly_add(&a, &b, &res);
        TEST_CHECK(fabs(res.coeffs[0] - 3.0) < TEST_EPS &&
                   fabs(res.coeffs[1] - 1.0) < TEST_EPS &&
                   fabs(res.coeffs[2] - 2.0) < TEST_EPS,
                   "add с нулевым: результат совпадает с a",                           ptr_ts);
        poly_free(&a); poly_free(&b); poly_free(&res);
    }

    /* Ошибочный случай: NULL */
    {
        status = poly_add(NULL, NULL, NULL);
        TEST_CHECK(status == POLY_ERROR_NULL, "add NULL -> POLY_ERROR_NULL", ptr_ts);
    }
}


static void test_sub(test_state_t *ptr_ts)
{
    polynomial_t a, b, res;
    poly_status_t status;

    printf("\n[Группа] poly_sub\n");

    /* Типовой случай: (3+4x) - (1+2x) = 2+2x */
    {
        poly_create(&a, 1);
        poly_create(&b, 1);
        a.coeffs[0] = 3.0; a.coeffs[1] = 4.0;
        b.coeffs[0] = 1.0; b.coeffs[1] = 2.0;
        status = poly_sub(&a, &b, &res);
        TEST_CHECK(status == POLY_OK,                    "sub типовой: OK",          ptr_ts);
        TEST_CHECK(fabs(res.coeffs[0] - 2.0) < TEST_EPS &&
                   fabs(res.coeffs[1] - 2.0) < TEST_EPS,
                   "sub типовой: коэффициенты",                                      ptr_ts);
        poly_free(&a); poly_free(&b); poly_free(&res);
    }

    /* Граничный случай: вычитание многочлена из самого себя → нулевой */
    {
        poly_create(&a, 2);
        a.coeffs[0] = 5.0; a.coeffs[1] = 3.0; a.coeffs[2] = 1.0;
        status = poly_sub(&a, &a, &res);
        TEST_CHECK(fabs(res.coeffs[0]) < TEST_EPS &&
                   fabs(res.coeffs[1]) < TEST_EPS &&
                   fabs(res.coeffs[2]) < TEST_EPS,
                   "sub a-a: все коэффициенты нулевые",                              ptr_ts);
        poly_free(&a); poly_free(&res);
    }

    /* Граничный случай: разные степени */
    {
        poly_create(&a, 3);
        poly_create(&b, 1);
        a.coeffs[0] = 1.0; a.coeffs[1] = 1.0; a.coeffs[2] = 0.0; a.coeffs[3] = 1.0;
        b.coeffs[0] = 1.0; b.coeffs[1] = 1.0;
        status = poly_sub(&a, &b, &res);
        TEST_CHECK(res.degree == 3,                      "sub разные степени: degree", ptr_ts);
        TEST_CHECK(fabs(res.coeffs[0]) < TEST_EPS &&
                   fabs(res.coeffs[1]) < TEST_EPS &&
                   fabs(res.coeffs[3] - 1.0) < TEST_EPS,
                   "sub разные степени: коэффициенты",                               ptr_ts);
        poly_free(&a); poly_free(&b); poly_free(&res);
    }

    /* Ошибочный случай: NULL */
    {
        status = poly_sub(NULL, NULL, NULL);
        TEST_CHECK(status == POLY_ERROR_NULL, "sub NULL -> POLY_ERROR_NULL", ptr_ts);
    }
}


static void test_scale(test_state_t *ptr_ts)
{
    polynomial_t p;
    poly_status_t status;

    printf("\n[Группа] poly_scale\n");

    /* Типовой случай: (1+2x+3x^2) * 2 = 2+4x+6x^2 */
    {
        poly_create(&p, 2);
        p.coeffs[0] = 1.0; p.coeffs[1] = 2.0; p.coeffs[2] = 3.0;
        status = poly_scale(&p, 2.0);
        TEST_CHECK(status == POLY_OK,                    "scale типовой: OK",         ptr_ts);
        TEST_CHECK(fabs(p.coeffs[0] - 2.0) < TEST_EPS &&
                   fabs(p.coeffs[1] - 4.0) < TEST_EPS &&
                   fabs(p.coeffs[2] - 6.0) < TEST_EPS,
                   "scale типовой: коэффициенты",                                    ptr_ts);
        poly_free(&p);
    }

    /* Граничный случай: умножение на 0 → нулевой многочлен */
    {
        poly_create(&p, 2);
        p.coeffs[0] = 9.0; p.coeffs[1] = 9.0; p.coeffs[2] = 9.0;
        poly_scale(&p, 0.0);
        TEST_CHECK(fabs(p.coeffs[0]) < TEST_EPS &&
                   fabs(p.coeffs[1]) < TEST_EPS &&
                   fabs(p.coeffs[2]) < TEST_EPS,
                   "scale на 0: все коэффициенты нулевые",                           ptr_ts);
        poly_free(&p);
    }

    /* Граничный случай: умножение на -1 */
    {
        poly_create(&p, 1);
        p.coeffs[0] = 3.0; p.coeffs[1] = -5.0;
        poly_scale(&p, -1.0);
        TEST_CHECK(fabs(p.coeffs[0] + 3.0) < TEST_EPS &&
                   fabs(p.coeffs[1] - 5.0) < TEST_EPS,
                   "scale на -1: знаки инвертированы",                               ptr_ts);
        poly_free(&p);
    }

    /* Ошибочный случай: NULL */
    {
        status = poly_scale(NULL, 2.0);
        TEST_CHECK(status == POLY_ERROR_NULL, "scale NULL -> POLY_ERROR_NULL", ptr_ts);
    }
}


static void test_derivative(test_state_t *ptr_ts)
{
    polynomial_t p, res;
    poly_status_t status;

    printf("\n[Группа] poly_derivative\n");

    /* Типовой случай: d/dx(1 + 2x + 3x^2) = 2 + 6x */
    {
        poly_create(&p, 2);
        p.coeffs[0] = 1.0; p.coeffs[1] = 2.0; p.coeffs[2] = 3.0;
        status = poly_derivative(&p, &res);
        TEST_CHECK(status == POLY_OK,                    "derivative типовой: OK",       ptr_ts);
        TEST_CHECK(res.degree == 1,                      "derivative типовой: degree==1", ptr_ts);
        TEST_CHECK(fabs(res.coeffs[0] - 2.0) < TEST_EPS &&
                   fabs(res.coeffs[1] - 6.0) < TEST_EPS,
                   "derivative типовой: коэффициенты",                                  ptr_ts);
        poly_free(&p); poly_free(&res);
    }

    /* Граничный случай: d/dx(константа) = 0 */
    {
        poly_create(&p, 0);
        p.coeffs[0] = 42.0;
        status = poly_derivative(&p, &res);
        TEST_CHECK(status == POLY_OK,                    "derivative константа: OK",       ptr_ts);
        TEST_CHECK(res.degree == 0,                      "derivative константа: degree==0", ptr_ts);
        TEST_CHECK(fabs(res.coeffs[0]) < TEST_EPS,       "derivative константа: coeffs==0", ptr_ts);
        poly_free(&p); poly_free(&res);
    }

    /* Граничный случай: d/dx(x^3) = 3x^2 */
    {
        poly_create(&p, 3);
        p.coeffs[3] = 1.0;
        status = poly_derivative(&p, &res);
        TEST_CHECK(res.degree == 2,                      "derivative x^3: degree==2",      ptr_ts);
        TEST_CHECK(fabs(res.coeffs[0]) < TEST_EPS &&
                   fabs(res.coeffs[1]) < TEST_EPS &&
                   fabs(res.coeffs[2] - 3.0) < TEST_EPS,
                   "derivative x^3: coeffs[2]==3",                                       ptr_ts);
        poly_free(&p); poly_free(&res);
    }

    /* Ошибочный случай: NULL */
    {
        status = poly_derivative(NULL, &res);
        TEST_CHECK(status == POLY_ERROR_NULL, "derivative NULL poly -> POLY_ERROR_NULL", ptr_ts);
    }

    /* Ошибочный случай: NULL result */
    {
        poly_create(&p, 2);
        status = poly_derivative(&p, NULL);
        TEST_CHECK(status == POLY_ERROR_NULL, "derivative NULL result -> POLY_ERROR_NULL", ptr_ts);
        poly_free(&p);
    }
}


int main(void)
{
    test_state_t ts = {0, 0};

    test_create_free(&ts);
    test_evaluate(&ts);
    test_add(&ts);
    test_sub(&ts);
    test_scale(&ts);
    test_derivative(&ts);

    printf("\nИтого: %zu прошло, %zu упало\n", ts.passed, ts.failed);

    return ts.failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
