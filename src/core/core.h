#ifndef _CORE_H_
#define _CORE_H_

#define NOP 0
#define NUMBER 1
#define UNARY 2
#define BINARY 3
#define INPUT 4

struct Symbol {
    char type;
    union {
        char unary;
        char binary;
        double number;
    } data;
};

extern struct Symbol expression[100];
extern double pi;
extern const char *unary_names[12];
extern const char *binary_names[5];

int core_evaluate(double in, double *out);
int core_integrate(double from, double to, unsigned long chunk, double *out);

#endif
