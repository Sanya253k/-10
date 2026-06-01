#include "../include/polynomial.h"

#include <math.h>
#include <stdlib.h>

poly_status_t poly_create(polynomial_t *poly, size_t degree){
	poly_status_t status = POLY_OK;

	if (poly == NULL) status = POLY_ERROR_NULL;
	else{
		poly -> coeffs = calloc(degree + 1, sizeof(double));
		if (poly -> coeffs == NULL) status = POLY_ERROR_MEMORY;
		else poly -> degree = degree;
	}
	return status;
}

void poly_free(polynomial_t *poly){
	if (poly != NULL){
		free(poly -> coeffs);
		poly -> coeffs = NULL;
		poly -> degree = 0;
	}
}

poly_status_t poly_evaluate(
		const polynomial_t *poly,
		double x,
		double *result){
	poly_status_t status = POLY_OK;
	double sum = 0.;
	if (poly == NULL || result == NULL) status = POLY_ERROR_NULL;
	else if (poly -> coeffs == NULL)   status = POLY_ERROR_NULL;
	else{
		for (size_t iter = 0; iter <= poly -> degree; iter++){
			sum += poly -> coeffs[iter] * pow(x, (double)iter);
		}
		*result = sum;
	}
	return status;
}

poly_status_t poly_add(
		const polynomial_t *a,
		const polynomial_t *b,
		polynomial_t *result){
	poly_status_t status = POLY_OK;
	size_t max_degree = 0;
	if (a == NULL || b == NULL || result == NULL) status = POLY_ERROR_NULL;
	else if (a -> coeffs == NULL || b -> coeffs == NULL) status = POLY_ERROR_NULL;
	else{
		max_degree = (a -> degree > b -> degree)
			? a -> degree
			: b -> degree;

		status = poly_create(result, max_degree);
		
		if (status == POLY_OK){
			for (size_t iter = 0; iter <= max_degree; iter++){
				double av = 0.;
				double bv = 0.;
				
				if (iter <= a -> degree) av = a -> coeffs[iter];
				if (iter <= b -> degree) bv = b -> coeffs[iter];

				result -> coeffs[iter] = av + bv;
			}
		}
	}
	
	return status;
}

poly_status_t poly_sub(
		const polynomial_t *a,
		const polynomial_t *b,
		polynomial_t *result){
	poly_status_t status = POLY_OK;
	size_t max_degree = 0;

	if (a == NULL || b == NULL || result == NULL) status = POLY_ERROR_NULL;
	else if (a -> coeffs == NULL || b -> coeffs == NULL) status = POLY_ERROR_NULL;
	else {
		max_degree = (a -> degree > b -> degree)
			? a -> degree
			: b -> degree;

		status = poly_create(result, max_degree);

		if (status == POLY_OK){
			for (size_t iter = 0; iter <= max_degree; iter++){
				double av = 0.;
				double bv = 0.;

				if (iter <= a -> degree) av = a -> coeffs[iter];
				if (iter <= b -> degree) bv = b -> coeffs[iter];

				result -> coeffs[iter] = av - bv;

			}
		}
	}
	return status;
}
