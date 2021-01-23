#include <math.h>
#include "core.h"

double pi = 3.14159265358979323846;
struct Symbol expression[100];

static double add(double a, double b) {
    return a + b;
}

static double subtract(double a, double b) {
    return a - b;
}

static double multiply(double a, double b) {
    return a * b;
}

static double divide(double a, double b) {
    return a / b;
}

static double (*unary_lookup[12])(double) = {
    sqrt,
    exp,
    exp2,
    log,
    log10,
    log2,
    sin,
    cos,
    tan,
    sinh,
    cosh,
    tanh
};

const char *unary_names[12] = {
    "sqrt",
    "exp",
    "exp2",
    "log",
    "log10",
    "log2",
    "sin",
    "cos",
    "tan",
    "sinh",
    "cosh",
    "tanh"
};

static double (*binary_lookup[5])(double, double) = {
    add,
    subtract,
    multiply,
    divide,
    pow
};

const char *binary_names[5] = {
    "+",
    "-",
    "*",
    "/",
    "^"
};

int core_evaluate(double in, double *out) {
    static double stack[100];
    double *sp = stack;
    if (!out) {
        return 1;
    }
    struct Symbol *ii = expression;
    for (char i = 0; i < 100; ++i, ++ii) {
        switch (ii->type) {
        case NUMBER:
            *(sp++) = ii->data.number;
            break;
        case UNARY:
            if (sp == stack) {
                return 2;
            }
            sp[-1] = unary_lookup[ii->data.unary](sp[-1]);
            break;
        case BINARY:
            if (sp - stack < 2) {
                return 2;
            }
            sp[-2] = binary_lookup[ii->data.binary](sp[-2], sp[-1]);
            --sp;
            break;
        case INPUT:
            *(sp++) = in;
            break;
        }
    }
    if (sp == stack) {
        return 3;
    }
    *out = sp[-1];
    return 0;
}

int core_integrate(double from, double to, unsigned long chunk, double *out) {
    if (!out) {
        return 1;
    }
    if (from > to)  {
        return 2;
    }
    double ii = from;
    double step = (to - from) / chunk;
    double sum = 0;
    double temp1, temp2;
    int ret;
    for (unsigned long i = 0; i < chunk; ++i, ii += step) {
        ret = core_evaluate(ii, &temp1);
        if (ret) {
            return ret + 1;
        }
        ret = core_evaluate(ii + step, &temp2);
        sum += (temp1 + temp2) * step / 2;
    }
    *out = sum;
    return 0;
}
