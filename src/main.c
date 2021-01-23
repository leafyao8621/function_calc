#include <stdio.h>
#include "core/core.h"

int main(void) {
    expression[0].type = NUMBER;
    expression[0].data.number = pi;
    expression[1].type = INPUT;
    expression[2].type = NUMBER;
    expression[2].data.number = 4;
    expression[3].type = BINARY;
    expression[3].data.binary = 4;
    expression[4].type = UNARY;
    expression[4].data.unary = 0;
    expression[5].type = BINARY;
    expression[5].data.binary = 0;
    double out = 0;
    for (int i = -100; i < 100; ++i) {
        core_evaluate(i, &out);
        printf("in: %d, out: %.6E\n", i, out);
    }
    core_integrate(0, 1, 10000, &out);
    printf("from: 0, to: 1, out: %.6E\n", out);
    return 0;
}
