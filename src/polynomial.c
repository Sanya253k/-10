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
