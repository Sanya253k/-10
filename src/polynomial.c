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
