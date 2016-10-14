#ifndef PNM_H
#define PNM_H
#include <stdio.h>

/**
 * Codigos de error para biblioteca de entrada/salida imagees
 */
typedef enum error_code {
    PNM_OK=0,
    PNM_ERROR_LECTURA=1,
    PNM_ARCHIVO_INEXISTENTE=2,
    PNM_ENCABEZADO_INVALIDO=3,
    PNM_INVALID_DATA=4,
    PNM_ERROR_ESCRITURA=5,
    PNM_FORMATO_INVALIDO=6
} ErrorCode;


const char* mensaje_error(ErrorCode c);

/**
 * Types de image representadas
 */
typedef enum image_type {
    BIN_ASC=1,
    GRISES_ASC=2,
    BIN_BIN=4,
    GRISES_BIN=5,
    COLOR_ASC=3,
    COLOR_BIN=6
} ImageType;

/**
 * Canales de color
 */
typedef enum canal {
    ROJO=0,
    VERDE=1,
    AZUL=2
} Canal;

/**
 * pixel es un entero sin signo de al menos 32 bits
 */
typedef unsigned int Pixel;

typedef struct image {
    ImageType type;
    int cols;
    int rows;
    int maxval;
    Pixel* pixels;
    int npixels;
} Image;

/**
 * Dados un cols y un rows, y una estructura de Image, reserva espacio en memoria para cols*rows pixels en la image.
 */
int init_image(int cols, int rows, ImageType type, Image* pimg);

int is_color(Image* img);

void alloc_image(Image* pimg);

void erase_image(Image* pimg);

/**
 * Crea una image de mismos atributos que la original
 */
int clone_image(const Image* source, Image* dest);

/**
 * Libera la memoria asociada a los pixels de la image dada.
 */
void destroy_image(Image* pimg);

ErrorCode read_header(FILE* fimg, Image* pimg);

ErrorCode read_data(FILE* fimg,Image* pimg);

/**
 * Carga una image PNM desde  un archivo dado.
 */
ErrorCode read_image(const char* path, Image* pimg);

/**
 * Escribe una image PNM desde en un archivo en la path especificada.
 */
ErrorCode write_image(const Image* pimg, const char* path);

#endif
