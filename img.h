
#ifndef DUDELIB_H
#define DUDELIB_H

typedef unsigned short val_t;
typedef int index_t;
typedef unsigned long count_t;

/**
 * pixel es un entero sin signo de al menos 32 bits
 */
typedef unsigned int Pixel;

typedef struct image {
    int type;
    int cols;
    int rows;
    int maxval;
    Pixel* pixels;
    int npixels;
} Image;


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

/*---------------------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------------------*/

/**
 * Dados un cols y un rows, y una estructura de Image, reserva espacio en memoria para cols*rows pixels en la image.
 */
int init_image(int cols, int rows, int type, Image* pimg);

int is_color(Image* img);

void alloc_image(Image* pimg);

void erase_image(Image* pimg);

/**
 * Crea una image de mismos atributos que la original
 */
void copy_image(Image* src, Image* dest);

int clone_image(const Image* source, Image* dest);

void decompose_image(const Image* I, Image* R, Image* G, Image* B);

void compose_image(const Image* R, const Image* G, const Image* B, Image* I);

/**
 * Libera la memoria asociada a los pixels de la image dada.
 */
void destroy_image(Image* pimg);

int get_pixel(const Image* pimg, int i, int j);

int get_pixel_circular(const Image* pimg, int i, int j);

int get_linear_pixel(const Image* pimg, int li);

void set_pixel(Image* pimg, int i, int j, Pixel x);

void set_linear_pixel(Image* pimg, int li, Pixel x);

#endif
