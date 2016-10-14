
#ifndef DUDELIB_H
#define DUDELIB_H
#include "pnm.h"


typedef unsigned short val_t;
typedef int index_t;
typedef unsigned long count_t;

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

/** A basic matrix */
typedef struct {
    double* values; /* MxN, in C order */
    unsigned int nrows;
    unsigned int ncols;
} Matrix;

/** A basic vector */
typedef struct {
    double* values;
    unsigned int n;
} Vector;

/**
 * Context realization
 */
typedef struct {
    val_t* values;
    unsigned int k;
} Context;

/**
 * Strategy for mapping contexts
 */
typedef void (*ContextMapper)(const Context* orig_ctx, Context* mapped_ctx);

/*---------------------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------------------*/

int get_pixel(const Image* pimg, int i, int j);

int get_linear_pixel(const Image* pimg, int li);

void set_pixel(Image* pimg, int i, int j, Pixel x);

void set_linear_pixel(Image* pimg, int li, Pixel x);

void read_template(const char* fname, Template* ptpl);

void print_template(const Template* ptpl);

void read_template_multi(const char* fname, Template** ptpls, int* ntemplates);

Template* ini_template(Template* pt, int k);

void destroy_template(Template* pt);

Template* generate_random_template(int max_l1_radius, int k, Template* pt);

void linearize_template(const Template* pt, int nrows, int ncols, const LinearTemplate* plt);

#endif
