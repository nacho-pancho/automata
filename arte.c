#include <stdlib.h>
#include "pnm.h"
#include "img.h"
#include "galois.h"

const char * ifname = "camera.pgm";
const char * ofbasename = "arte2_%03d.pgm";

typedef Pixel (*rule_t)(Image* img, Image* S0,Image* S1);

Pixel rule0(Image* img, Image* S0, Image* S1);
Pixel rule1(Image* img, Image* S0, Image* S1);
Pixel rule2(Image* img, Image* S0, Image* S1);
Pixel rule3(Image* img, Image* S0, Image* S1);
Pixel rule4(Image* img, Image* S0, Image* S1);
Pixel rule5(Image* img, Image* S0, Image* S1);
Pixel rule6(Image* img, Image* S0, Image* S1);
Pixel rule7(Image* img, Image* S0, Image* S1);
Pixel rule8(Image* img, Image* S0, Image* S1);
Pixel rule9(Image* img, Image* S0, Image* S1);
Pixel rule10(Image* img, Image* S0, Image* S1);

rule_t rules[] = {rule0, rule1, rule2, rule3, rule4, rule5, rule6, rule7, rule8};

char* progname;
char const *tplfile = "template.asc";
double thres = 0.95;
unsigned long seed = 55627647UL;
int ruleno = 1;
int epochs = 1;
int movie = 0;
Template* tpls;
int ntemplates = 10;
int tplsize = 10;
int maxradius = 7;
int mix = 1;
int qbits = 0;

void parse_options(int* pargc, char** pargv[]);

int main(int argc, char** argv) {
  FILE* fimg;
  Image I,S0,S1;
  char ofname[128];
  int maxbits;
  parse_options(&argc,&argv);
  if (argc > 0) { ifname = argv[0]; }
  if (argc > 1) { ofbasename = argv[1]; }
  fprintf(stdout,"*input=%s\n",ifname);
  fprintf(stdout,"*output=%s\n",ofbasename);
  fprintf(stdout,"*rule=%d\n",ruleno);
  fprintf(stdout,"*qbits=%d\n",qbits);
  fprintf(stdout,"*epochs=%d\n*movie=%d\n*mix=%d\n",epochs,movie,mix);
  fimg = fopen(ifname,"r");
  if (!fimg) {
        fprintf(stderr,"Error opening %s for reading.",ifname);
      return -1;
    }
  read_header(fimg,&I);
  const int rows = I.rows;
  const int cols = I.cols;
  const int npixels = rows*cols;
  maxbits = 1;
  for (int i = I.maxval; i >>= 1; maxbits++) {}
  fprintf(stdout,"Input image rows=%d cols=%d maxval=%d maxbits=%d\n",rows,cols,I.maxval,maxbits);
  alloc_image(&I);
  read_data(fimg,&I);
  //
  // if qbits > 0, quantize image
  //
  if (qbits >0) {
    for (int li = 0; li < npixels; li++) {
      I.pixels[li] >>= qbits;
    }
    I.maxval >>= qbits;
    maxbits -= qbits;
    fprintf(stdout,"-after quantization maxval=%d maxbits=%d\n",I.maxval,maxbits);
  }
  gf_ini(maxbits);
  clone_image(&I,&S0);
  clone_image(&I,&S1);
  fclose(fimg);
  srand(seed);
  //
  // initialize state
  //

  const int maxplane = is_color(&I)? 24: 8;
  for (int i = 0, li = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++,li++) {
      for (int p = 0; p < maxplane; p++) {
        int t = ((double)rand()/(double)RAND_MAX) > thres;
        S0.pixels[li] |=  (t << p);
      }
    }
  }
  rule_t rule = rules[ruleno];
  unsigned maxx = 0;
  for (int e = 0; e < epochs; e++) {
      maxx = rule(&I,&S0,&S1);
#if 0
           if (mix) {
               int y = I.pixels[li];
               if (!is_color(&O)) {
                 O.pixels[li] = x*y/255;
                }  else {
                    int r = (y >> 16) & 0xff;
                    int g = (y >>  8) & 0xff;
                    int b = (y      ) & 0xff;
                    int rx = (x >> 16) & 0xff;
                    int gx = (x >>  8) & 0xff;
                    int bx = (x      ) & 0xff;
                    int ro = (rx*r) >> 8;
                    int go = (gx*g) >> 8;
                    int bo = (bx*b) >> 8;
                    O.pixels[li] = (ro << 16) + (go << 8) + bo;
                }
            } else {
                O.pixels[li] = x;
            }
#endif
    snprintf(ofname,128,ofbasename,e);
    fprintf(stdout,"-epoch %d output %s maxval %d\n",e,ofname,maxx);
    if (movie || (e==(epochs-1))) write_image(&S1,ofname);
    // circular buffer: swap prev and curr states
    Pixel* aux = S1.pixels;
    S1.pixels = S0.pixels;
    S0.pixels = aux;
  }
  fprintf(stdout,"cleanup.\n");
  destroy_image(&S1);
  destroy_image(&S0);
  destroy_image(&I);
  fprintf(stdout,"done.\n");
}


void parse_options(int* pargc, char** pargv[]) {
    int argc = *pargc;
    char** argv = *pargv;
    progname = argv[0];
    for (++argv,--argc; (argc > 0) && *argv[0] == '-'; --argc, ++argv) {
        switch (argv[0][1]) {
        case 'm':
            if (argv[0][2]) // parameter val is glued , -pval
                movie = atof(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                movie = atof((++argv)[0]);
            printf("movie=%d\n",movie);
            break;
         case 'e':
            if (argv[0][2]) // parameter val is glued , -pval
                epochs = atof(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                epochs = atof((++argv)[0]);
            printf("epochs=%d\n",epochs);
            break;
        case 't':
            if (argv[0][2]) // parameter val is glued , -pval
                thres = atof(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                thres = atof((++argv)[0]);
            printf("thres=%f\n",thres);
            break;
        case 's':
            if (argv[0][2]) // parameter val is glued , -pval
                seed = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                seed = atoi((++argv)[0]);
            printf("seed=%lu\n",seed);
            break;
        case 'r':
            if (argv[0][2]) // parameter val is glued , -pval
                ruleno = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                ruleno = atoi((++argv)[0]);
            printf("ruleno=%d\n",ruleno);
            break;
        case 'T':
            if (argv[0][2]) // parameter val is glued , -tfilename
                tplfile = argv[0]+2;
            else if (argc-- > 0) // parameter val is next arg
                tplfile = (++argv)[0];
            printf("tplfile=%s\n",tplfile);
            break;
        case 'K':
            if (argv[0][2]) // parameter val is glued , -pval
                tplsize = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                tplsize = atoi((++argv)[0]);
            printf("tplsize=%d\n",tplsize);
            break;
        case 'q':
            if (argv[0][2]) // parameter val is glued , -pval
                qbits = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                qbits = atoi((++argv)[0]);
            printf("qbits=%d\n",qbits);
            break;
        case 'R':
            if (argv[0][2]) // parameter val is glued , -pval
                maxradius = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                maxradius = atoi((++argv)[0]);
            printf("maxradius=%d\n",maxradius);
            break;
        case 'x':
            if (argv[0][2]) // parameter val is glued , -pval
                mix = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                mix = atoi((++argv)[0]);
            printf("mix=%d\n",mix);
            break;
        default:
            printf("Unknown option %s\n",argv[0]);
        }
    }
    *pargc = argc;
    *pargv = argv;
}

//
// basic 3 bit rule, rule defined by image
//
Pixel rule0(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xl = get_pixel(S0,i,j-1) != 0;
  const int xc = get_pixel(S0,i,j) != 0;
  const int xr = get_pixel(S0,i,j+1) != 0;
  // the rule is given by the Image value
  // a rule here is a table. The i-th bit in the rule is the output
  // of the 3-neighbor input (xl,xc,xr) indexed lexicographically
  // by i=4*xl+2*xc+xr
  const int rule = get_pixel(img,i,j);
  const int ri = (((xl << 1) | xc) << 1) | xr;
  const int newx = (rule >> ri) & 0x01; //
  set_pixel(S1,i+1,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

//
// rule applied on each bit plane
//
Pixel rule1(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xl = get_pixel(S0,i,j-1);
  const int xc = get_pixel(S0,i,j);
  const int xr = get_pixel(S0,i,j+1);
  int rule = get_pixel(img,i,j);
  //if (rule == 0) rule = 30;
  //if (rule == 255) rule = 110;
  //const int rule = 30;
  unsigned newx = 0;
  const int maxplane = is_color(img)? 24: 8;
  for (int p = 0; p < maxplane; p++) {
  const int yl = (xl>>p) & 0x01;
  const int yc = (xc>>p) & 0x01;
  const int yr = (xr>>p) & 0x01;
  const int ri = (((yl << 1) | yc) << 1) | yr;
  const int sp = (rule >> ri) & 0x01;
  newx |= (sp<<p);
  }
  // the rule is given by the Image value
  // a rule here is a table. The i-th bit in the rule is the output
  // of the 3-neighbor input (xl,xc,xr) indexed lexicographically
  // by i=4*xl+2*xc+xr
  set_pixel(S0,i+1,j,newx);
  set_pixel(S1,i+1,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

//
// same as rule1 but uses horizontal gradient of image as rule
//
Pixel rule2(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xl = get_pixel(S0,i,j-1);
  const int xc = get_pixel(S0,i,j);
  const int xr = get_pixel(S0,i,j+1);
  int rule = (get_pixel(img,i,j) - get_pixel(img,i,j)) / 2;
  if (rule < 0) rule = -rule;
  unsigned newx = 0;
  const int maxplane = is_color(img)? 24: 8;
  for (int p = 0; p < maxplane; p++) {
  const int yl = (xl>>p) & 0x01;
  const int yc = (xc>>p) & 0x01;
  const int yr = (xr>>p) & 0x01;
  const int ri = (((yl << 1) | yc) << 1) | yr;
  const int sp = (rule >> ri) & 0x01;
  newx |= (sp<<p);
  }
  set_pixel(S1,i+1,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

//
// first attempt at continuos rule
//
Pixel rule3(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xl = get_pixel(S0,i,j-1) != 0;
  const int xc = get_pixel(S0,i,j) != 0;
  const int xr = get_pixel(S0,i,j+1) != 0;
  const int rule = get_pixel(img,i,j);
  const int ri = (((xl << 1) | xc) << 1) | xr;
  const int ruleval = (rule >> ri) & 0x01;
  int prev = get_pixel(S0,i,j);
  int newx = ruleval ? (prev+1) : (prev-1);
  newx  = newx > 255 ? 255: (newx < 0 ? 0: newx);
  set_pixel(S1,i+1,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

//
// another continuous rule, quite crazy
//
Pixel rule4(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xl = get_pixel(S0,i,j-1) != 0;
  const int xc = get_pixel(S0,i,j) != 0;
  const int xr = get_pixel(S0,i,j+1) != 0;
  const int rule = get_pixel(img,i,j);
  const int ri = (((xl << 1) | xc) << 1) | xr;
  const int ruleval = (rule >> ri) & 0x01; //
  int newx = ruleval ? (get_pixel(S0,i,j)>>1) : (get_pixel(S0,i,j)<<1);
  newx  = newx > 255 ? 1: (newx <= 0 ? 128: newx);
  set_pixel(S1,i+1,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

//
// Conway's game of life modulated by image
//
Pixel rule5(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xw =  get_pixel(S0,i  ,j-1) != 0;
  const int xc =  get_pixel(S0,i  ,j  ) != 0;
  const int xe =  get_pixel(S0,i  ,j+1) != 0;
  const int xn =  get_pixel(S0,i-1,j  ) != 0;
  const int xs =  get_pixel(S0,i+1,j  ) != 0;
  const int xnw = get_pixel(S0,i-1,j-1) != 0;
  const int xne = get_pixel(S0,i-1,j+1) != 0;
  const int xsw = get_pixel(S0,i+1,j-1) != 0;
  const int xse = get_pixel(S0,i+1,j+1) != 0;
  const int yc = get_pixel(img,i  ,j  ) > 128;
  int n = xw + xe + xn + xs + xnw + xne + xsw + xse + yc;
  int newx = xc ? 1: 0;
  if ((n < 2) || (n > 3)) {
    newx = 0;
  } else if (n == 3) {
    newx = 1;
  }
  set_pixel(S1,i,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

//
// bitplane version of rule 5
//
Pixel rule6(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xw =  get_pixel(S0,i  ,j-1);
  const int xc =  get_pixel(S0,i  ,j  );
  const int xe =  get_pixel(S0,i  ,j+1);
  const int xn =  get_pixel(S0,i-1,j  );
  const int xs =  get_pixel(S0,i+1,j  );
  const int xnw = get_pixel(S0,i-1,j-1);
  const int xne = get_pixel(S0,i-1,j+1);
  const int xsw = get_pixel(S0,i+1,j-1);
  const int xse = get_pixel(S0,i+1,j+1);
  const int yc = get_pixel(img,i  ,j  );
  int newx = 0;
  const int maxplane = is_color(img)? 24: 8;
  for (int p = 0; p < maxplane; p++) {
    int n = ((xw>>p)&1) + ((xe>>p)&1) + ((xn>>p)&1) + ((xs>>p)&1) +
    ((xnw>>p)&1) + ((xne>>p)&1) + ((xsw>>p)&1) + ((xse>>p)&1) + ((yc>>p)&1);
    int b = xc & (1<<p);
    if ((n < 2) || (n > 3)) {
      b = 0;
    } else if (n == 3) {
      b = (1<<p);
    }
    newx |= b;
  }
  set_pixel(S1,i,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

Pixel rule7(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xw =  get_pixel(S0,i  ,j-1);
  const int xc =  get_pixel(S0,i  ,j  );
  const int xe =  get_pixel(S0,i  ,j+1);
  const int xn =  get_pixel(S0,i-1,j  );
  const int xs =  get_pixel(S0,i+1,j  );
  const int xnw = get_pixel(S0,i-1,j-1);
  const int xne = get_pixel(S0,i-1,j+1);
  const int xsw = get_pixel(S0,i+1,j-1);
  const int xse = get_pixel(S0,i+1,j+1);
  const int yc = get_pixel(img,i  ,j  );
  int newx = 0;
  const int maxplane = is_color(img)? 24: 8;
  for (int p = 0; p < maxplane; p++) {
    int n = ((xw>>p)&1) + ((xe>>p)&1) + ((xn>>p)&1) + ((xs>>p)&1) +
    ((xnw>>p)&1) + ((xne>>p)&1) + ((xsw>>p)&1) + ((xse>>p)&1);
    int b = xc & (1<<p);
    if ((n < (yc/64)) || (n >= (yc/32))) {
      b = 0;
    } else if ((n >=   ((3.0*yc)/128.0)) && (n < ((double)yc/32.0)) ) {
      b = (1<<p);
    }
    newx |= b;
  }
  set_pixel(S1,i,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}

Pixel rule8(Image* img, Image* S0, Image* S1) {
  const int cols = img->cols;
  const int rows = img->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const int xw =  get_pixel(S0,i  ,j-1);
  const int xc =  get_pixel(S0,i  ,j  );
  const int xe =  get_pixel(S0,i  ,j+1);
  const int xn =  get_pixel(S0,i-1,j  );
  const int xs =  get_pixel(S0,i+1,j  );
  const int xnw = get_pixel(S0,i-1,j-1);
  const int xne = get_pixel(S0,i-1,j+1);
  const int xsw = get_pixel(S0,i+1,j-1);
  const int xse = get_pixel(S0,i+1,j+1);
  const int yc = get_pixel(img,i  ,j  );
  int n = xw + xe + xn + xs + xnw + xne + xsw + xse - 2*yc;
  int newx = xc;
  if (n < 0) {
    newx--;
  } else if (n > 0) {
    newx++;
  }
  if (newx > 255) newx = 55;
  if (newx < 0) newx = 0;
  set_pixel(S1,i,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}
