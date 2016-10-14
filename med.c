#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "image.h"

#define MAXW 10

/*------------------------------------------------------------------------*/

static int clip(int i, int min, int max) {
    return  i <= min ? min : (i >= max ? max : i);
}

/*------------------------------------------------------------------------*/

static int pixel_comp(const void* p1, const void* p2) {
    return *((Pixel*)p1)-*((Pixel*)p2);
}

/*------------------------------------------------------------------------*/
void med(const Imagen* in, int w, Imagen* out) {
    const int ancho = in->ancho;
    const int alto =  in->alto;
    const int wsz = (w+1)*(w+1);
    const Pixel* pi = in->pixels;
    Pixel* po       = out->pixels;
    int i,j,di,dj,i2,j2,k;
    Pixel buffer[(MAXW+1)*(MAXW+1)]; /* maximo tamaño de ventana */

    printf("ancho=%d alto=%d\n",ancho,alto);
    for (i = 0; i < alto ; ++i) {
        for (j = 0 ; j < ancho; ++j) {
            k = 0;
            for (di = -w; di <= w ; ++di) {
                for (dj = -w ; dj <= w; ++dj) {
                    i2 = clip(i + di, 0, alto-1 );
                    j2 = clip(j + dj, 0, ancho-1 );
                    buffer[k++] = pi[i2*ancho+j2];
                }
            }
            qsort(buffer,wsz,sizeof(Pixel),pixel_comp);
            po[i*ancho+j] = (wsz % 2) == 0 ?
                            (buffer[wsz/2] + buffer[wsz/2-1])  / 2 :
                            buffer[wsz/2];
        }
    }
}
