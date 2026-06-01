#ifndef POLYNOMAL_H
#define POLYNOMAL_H

#include <stddef.h>

#ifdef __cplusplus

extern "C" {
#endif

	typedef enum {
		POLY_OK = 0,
		POLY_ERROR_NULL,
		POLY_ERROR_MEMORY,
		POLY_ERROR_SIZE
	} poly_status_t;

	typedef struct {
		double *coeffs;
		size_t degree;
	} polynomial_t;

	/**
	 * @brief  Создаем многочлен
	*/
	poly_status_t poly_create(polynomial_t *poly, size_t degree);

	/**
	 * @brief Освобождаем память
	 */
	void poly_free(polynomial_t *poly);

	/**
	 * @brief Вычислить значение многочлена
	 */
	poly_status_t poly_evaluate(
			const polynomial_t *poly,
			double x,
			double *result
	);

	/**
	 * @brief Сложение многочленов
	 */
	poly_status_t poly_add(
			const polynomial_t *a,
			const polynomial_t *b,
			polynomial_t *result
	);

	/**
	 * @brief Вычитание многочленов
	 */
	poly_status_t poly_sub(
			const polynomial_t *a,
			const polynomial_t *b,
			polynomial_t *result
	);

	/**
	 * @brief умножение на скаляр
	 */
	poly_status_t poly_scale(
			polynomial_t *poly,
			double scalar
	);

	/**
	 * @brief Производная
	 */
	poly_status_t poly_derivative(
			const polynomial_t *poly,
			polynomial_t *result
	);

#ifdef __cplusplus
}
#endif

#endif

