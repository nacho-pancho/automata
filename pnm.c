#include "pnm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char* error_messages[] = { "Todo bien","Error de lectura", "Archivo inexistente", "Encabezado invalido", "data invalidos", "Error de escritura" };

const char* error_message(ErrorCode c) {
    return error_messages[c];
}


static ErrorCode read_data_p2(FILE* fimg, Image* pimg);
static ErrorCode read_data_p4(FILE* fimg, Image* pimg);
static ErrorCode read_data_p5(FILE* fimg, Image* pimg);
static ErrorCode read_data_p6(FILE* fimg, Image* pimg);

ErrorCode read_data(FILE* fimg,Image* pimg) {
    switch (pimg->type) {
    case GRISES_ASC:
        return read_data_p2(fimg,pimg);
        break;
    case GRISES_BIN:
        return read_data_p5(fimg,pimg);
        break;
    case COLOR_BIN:
        return read_data_p6(fimg,pimg);
        break;
    case BIN_BIN:
        return read_data_p4(fimg,pimg);
        break;
    default:
        printf("Type de image %d no soportado.\n",pimg->type);
        return PNM_FORMATO_INVALIDO;
    }
}

ErrorCode read_image(const char* path, Image* pimg) {
    ErrorCode ret;
    /* abrimos archivo */
    FILE* fimg = fopen(path, "r");
    if (fimg == NULL) {
        return PNM_ARCHIVO_INEXISTENTE;
    }
    /* leemos encabezado */
    ret  = read_header(fimg,pimg);
    if (ret != PNM_OK) {
        fclose(fimg);
        return ret;
    }
    /* leemos data según type */
    init_image(pimg->cols,pimg->rows,pimg->type,pimg);
    read_data(fimg,pimg);
    if (ret != PNM_OK)
        destroy_image(pimg);
    fclose(fimg);
    return ret;
}

ErrorCode read_data_p2(FILE* fimg, Image* pimg) {
    register int i,aux;
    const int n = pimg->cols*pimg->rows;
    aux = 1;
    for (i = 0; (i < n) && (aux > 0); ++i) {
        aux = fscanf(fimg," %d",(pimg->pixels + i));
    }
    if (i < n)
        return PNM_INVALID_DATA;
    else
        return PNM_OK;
}

ErrorCode read_data_p4(FILE* fimg, Image* pimg) {
  register int i,li;
    const int m = pimg->rows;
    const int n = pimg->cols;
    for (i = 0, li=0; i < m; ++i) {
      unsigned short mask = 0;
      int j;
      short r = 0;
      for (j = 0; j < n; ++j, ++li) {
  if (!mask) {
    r = fgetc(fimg);
    mask = 0x80;
    if (r < 0) return PNM_INVALID_DATA;
  }
        pimg->pixels[li] = (r & mask) ? 1 : 0;
  mask >>= 1;
      }
    }
    if (li < n)
        return PNM_INVALID_DATA;
    else
        return PNM_OK;
}

ErrorCode read_data_p6(FILE* fimg, Image* pimg) {
    register int i, aux;
    unsigned char r,g,b;
    const int n = pimg->cols*pimg->rows;
    aux = 3;
    for (i = 0; (i < n) && (aux == 3); ++i) {
        aux = fread(&r,sizeof(char),1,fimg);    /* maximo asumido de 8 bits por canal! */
        aux += fread(&g,sizeof(char),1,fimg);
        aux += fread(&b,sizeof(char),1,fimg);
        pimg->pixels[i] = (r << 16) + (g << 8) + b;
    }
    if (i < n)
        return PNM_INVALID_DATA;
    else
        return PNM_OK;
}

ErrorCode read_data_p5(FILE* fimg, Image* pimg) {
    register int i;
    unsigned char p;
    const int n = pimg->cols*pimg->rows;
    for (i = 0; i < n; ++i) {
        int r = fread(&p,sizeof(char),1,fimg);    /* maximo asumido de 8 bits por canal! */
        if (!r) return PNM_INVALID_DATA;
        pimg->pixels[i] = p;
    }
    if (i < n)
        return PNM_INVALID_DATA;
    else
        return PNM_OK;
}

ErrorCode read_header(FILE* fimg, Image* pimg) {
    char magic;
    int cols,rows,maxval = 0, aux = 0;
    magic = fgetc(fimg);
    /* 1. numero magico */
    if (magic != 'P')
        return PNM_ENCABEZADO_INVALIDO;
    magic = fgetc(fimg);
    pimg->type = magic - '0';
    if (pimg->type >6)
        return PNM_ENCABEZADO_INVALIDO;

    /* cols */
    aux = fscanf(fimg," %d",&cols);
    if ((cols == 0) || (aux == 0))
        return PNM_ENCABEZADO_INVALIDO;
    /* rows */
    aux = fscanf(fimg," %d",&rows);
    if ((rows == 0) || (aux == 0))
        return PNM_ENCABEZADO_INVALIDO;
    if (pimg->type != 4) {
    /* maxval */
    aux = fscanf(fimg," %d",&maxval);
    if ((maxval == 0) || (aux == 0))
        return PNM_ENCABEZADO_INVALIDO;
    if ((maxval > 255) && (magic == '6'))
        return PNM_ENCABEZADO_INVALIDO;
    } else {
      maxval = 1;
    }
    /* un espacio al final */
    aux = fgetc(fimg);
    pimg->cols = cols;
    pimg->rows = rows;
    pimg->maxval = maxval;
    return PNM_OK;
}

ErrorCode write_data_p2(const Image* pimg, FILE* fimg);
ErrorCode write_data_p4(const Image* pimg, FILE* fimg);
ErrorCode write_data_p5(const Image* pimg, FILE* fimg);
ErrorCode write_data_p6(const Image* pimg, FILE* fimg);

/**
 * Escribe una image PNM desde en un archivo en la path especificada.
 */
ErrorCode write_image(const Image* pimg, const char* path) {
    FILE* fimg;
    int ret;
    fimg = fopen(path,"w");
    if (fimg == NULL)
        return PNM_ARCHIVO_INEXISTENTE;

    /* encabezado */
    if (pimg->type != 4)
      fprintf(fimg,"P%c %d %d\n%d\n",pimg->type+'0', pimg->cols, pimg->rows, pimg->maxval);
    else
      fprintf(fimg,"P%c %d %d\n",pimg->type+'0', pimg->cols, pimg->rows);
    switch (pimg->type) {
    case GRISES_ASC:
        ret = write_data_p2(pimg,fimg);
        break;
    case BIN_BIN:
        ret = write_data_p4(pimg,fimg);
        break;
    case GRISES_BIN:
        ret = write_data_p5(pimg,fimg);
        break;
    case COLOR_BIN:
        ret = write_data_p6(pimg,fimg);
        break;
    default:
        printf("Type de image %d no soportado.\n",pimg->type);
        ret = PNM_FORMATO_INVALIDO;
    }
    fclose(fimg);
    return ret;
}


ErrorCode write_data_p2(const Image* pimg, FILE* fimg) {
    /* data */
    const int cols = pimg->cols;
    const int rows = pimg->rows;
    register int i,j;
    Pixel* pix = pimg->pixels;
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j, ++pix) {
            fprintf(fimg," %d", *pix);
        }
        fputc('\n',fimg);
    }
    /* fin */
    return PNM_OK;
}

ErrorCode write_data_p4(const Image* pimg, FILE* fimg) {
    const int cols = pimg->cols;
    const int rows = pimg->rows;
    const Pixel* pixels = pimg->pixels;
    register int i,j;
    for (i = 0; i < rows; ++i) {
      unsigned short mask = 0x80;
      unsigned short word = 0x00;
      for (j = 0; j < cols; ++j,++pixels) {
        unsigned short p = *pixels;
  if (p)
    word |= mask;
  mask >>=1;
  if (!mask) {
    fputc(word, fimg);
    mask = 0x80;
    word = 0;
  }
      }
      if ((mask > 0) && (mask != 0x80))
          fputc(word, fimg);
    }
    return PNM_OK;
}

ErrorCode write_data_p5(const Image* pimg, FILE* fimg) {
    const int cols = pimg->cols;
    const int rows = pimg->rows;
    const int n = cols*rows;
    const Pixel* pixels = pimg->pixels;
    register int i;
    for (i = 0; i < n; ++i,++pixels) {
        int p = *pixels;
        fputc(p & 0xff, fimg);
    }
    return PNM_OK;
}

ErrorCode write_data_p6(const Image* pimg, FILE* fimg) {
    const int cols = pimg->cols;
    const int rows = pimg->rows;
    const int n = cols*rows;
    const Pixel* pixels = pimg->pixels;
    register int i;
    for (i = 0; i < n; ++i,++pixels) {
        int p = *pixels;
        fputc((p >> 16) & 0xff, fimg); /* r */
        fputc((p  >> 8) & 0xff, fimg); /* g */
        fputc((p      ) & 0xff, fimg); /* b */
    }
    return PNM_OK;
}


