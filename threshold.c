#include "image.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  int res;
  int i;
  Imagen img;
  int u = atoi(argv[1]);
  memset(&img,0,sizeof(img));
  printf("Leyendo image.\n");
  res = leer_image(argv[2],&img);
  if (res != PNM_OK) return res;
  printf("Escribiendo image.\n");
  const int n = img.alto*img.ancho;
  for (i = 0; i < n; i++) {
    img.pixels[i] = (img.pixels[i] >= u) ? 1 : 0;
  }
  img.tipo=4;
  res = escribir_image(&img,argv[3]);
  printf("Fin.\n");
  destruir_image(&img);
  return res;
}
