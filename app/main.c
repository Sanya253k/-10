#include <stdio.h>
#include "polynomial.h"

int main(void){
	polynomial_t p;
	polynomial_t p1;
	polynomial_t res;

	poly_create(&p, 2);
	poly_create(&p1,2);

	p.coeffs[0] = 1;
	p.coeffs[1] = 2;
	p.coeffs[2] = 3;

	p1.coeffs[0] = 1;
	p1.coeffs[1] = 2;
	p1.coeffs[2] = 3;

	poly_add(&p, &p1, &res);

	printf("result: %lf %lf %lf\n", res.coeffs[0], res.coeffs[1], res.coeffs[2]);

	poly_free(&p);
	poly_free(&p1);
	poly_free(&res);
	return 0;
}
