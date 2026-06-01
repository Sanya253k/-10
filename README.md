# polynomial

Native C library for polynomial arithmetic.

## Features

- Create and free polynomials
- Evaluate a polynomial at a given point
- Addition and subtraction of polynomials
- Scalar multiplication
- Symbolic differentiation

## Build

```bash
make all    # build shared library + demo app
make run    # run demo
make test   # run all tests
make clean  # remove build artifacts
```

## Structure

```
include/   — public headers
src/       — library source
app/       — demo application
tests/     — C and Python tests
```
