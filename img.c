#include "ascmat.h"
#include <stdlib.h>
#include "img.h"
#include "pnm.h"
#include "math.h"

#define LINEARIZE 0


/*---------------------------------------------------------------------------------------*/

void alloc_image(Image* pimg) {
    if (pimg->rows*pimg->cols > 0) {
        pimg->pixels = (Pixel*) malloc(pimg->cols*pimg->rows*sizeof(Pixel));
    } else{
        pimg->pixels = 0;
    }
}

/*---------------------------------------------------------------------------------------*/

/*
 * TIEMPOS:
 *   IMPLEMENTAR image.h + image.c : 1:00
 *   DEPURAR image.h + image.c     : 0:40
 */
int init_image(int cols, int rows, int type, Image* pimg) {
    if (rows*cols > 0) {
        pimg->pixels = (Pixel*) malloc(cols*rows*sizeof(Pixel));
        pimg->cols = cols;
        pimg->rows = rows;
        pimg->npixels = cols*rows;
        pimg->type = type;
  pimg->maxval = type == 4 ? 1 : 255;
        return (!pimg)? 1: 0;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------*/

int is_color(Image* img) {
    return (img->type == COLOR_ASC) || (img->type == COLOR_BIN);
}

/*---------------------------------------------------------------------------------------*/

void erase_image(Image* pimg) {
    memset(pimg->pixels,0,sizeof(Pixel)*pimg->rows*pimg->cols);
}


/*---------------------------------------------------------------------------------------*/

void copy_image(Image* src, Image* dest) {
    memcpy(dest->pixels,src->pixels,src->rows*src->cols*sizeof(Pixel));
}

/*---------------------------------------------------------------------------------------*/

int clone_image(const Image* po, Image* pd) {
    pd->maxval = po->maxval;
    return init_image(po->cols,po->rows,po->type,pd);
}

void decompose_image(const Image* I, Image* R, Image* G, Image* B) {
  const int npix = I->rows* I->cols;
  for (int i = 0; i < npix; ++i) {
    const int p = I->pixels[i];
    R->pixels[i] = (p >> 16)&0xff;
    G->pixels[i] = (p >> 8) & 0xff;
    B->pixels[i] = p & 0xff;
  }
}

void compose_image(const Image* R, const Image* G, const Image* B, Image* I) {
  const int npix = I->rows* I->cols;
  for (int i = 0; i < npix; ++i) {
    const int r = R->pixels[i];
    const int g = G->pixels[i];
    const int b = B->pixels[i];
    I->pixels[i] = (r<<16) | (g<<8) | b;
  }
}

/*---------------------------------------------------------------------------------------*/

void destroy_image(Image* pimg) {
    free(pimg->pixels);
    pimg->cols = 0;
    pimg->rows = 0;
    pimg->pixels = NULL;
}

/*---------------------------------------------------------------------------------------*/

int get_pixel(const Image* pimg, int i, int j) {
    return (i < 0) || (i >= pimg->rows) || (j < 0) || (j >= pimg->cols) ? 0 : pimg->pixels[i*pimg->cols+j];
}

int get_pixel_circular(const Image* pimg, int i, int j) {
    if (i < 0) i += pimg->rows;
    else if (i >= pimg->rows) i -= pimg->rows;
    if (j < 0) j += pimg->cols;
    else if (j >= pimg->cols) j -= pimg->cols;
    return pimg->pixels[i*pimg->cols+j];
}
/*---------------------------------------------------------------------------------------*/

int get_linear_pixel(const Image* pimg, int li) {
    //return pimg->pixels[(li + pimg->npixels) % (pimg->npixels)];   // MODULO
    return (li < 0) || (li >= pimg->npixels) ? 0 : pimg->pixels[li];
}

void set_pixel(Image* pimg, int i, int j, Pixel x) {
    if  ((i < 0) || (i >= pimg->rows) || (j < 0) || (j >= pimg->cols))
        return;
    else
        pimg->pixels[i*pimg->cols+j] = x;
}

/*---------------------------------------------------------------------------------------*/

void set_linear_pixel(Image* pimg, int li, Pixel x) {
    //return pimg->pixels[(li + pimg->npixels) % (pimg->npixels)];   // MODULO
    if  ((li < 0) || (li >= pimg->npixels) )
        return;
    else
        pimg->pixels[li] = x;
}

