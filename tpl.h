#ifndef TPL_H
#define TPL_H
#include "img.h"
#include <stdio.h>

/** 2D Template */
typedef struct {
    index_t* is;
    index_t* js;
    unsigned int k;
} Template;

/** Linear version of a template for faster access */
typedef struct {
    index_t* li;
    unsigned int k;
} LinearTemplate;

void read_template(const char* fname, Template* ptpl);

void print_template(const Template* ptpl);

void read_template_multi(const char* fname, Template** ptpls, int* ntemplates);

Template* ini_template(Template* pt, int k);

void destroy_template(Template* pt);

Template* generate_random_template(int radius, int norm, int k, int sym, Template* pt);

void linearize_template(const Template* pt, int nrows, int ncols, const LinearTemplate* plt);

void dump_template(const Template* ptpl,FILE* ft);

Template* generate_ball_template(int radius, int norm, Template* pt);

void symmetrize_template(const Template* in, Template* out);

#endif
