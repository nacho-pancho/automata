#include <stdlib.h>
#include "pnm.h"
#include "img.h"
#include "galois.h"
#include "tpl.h"
#include "context.h"

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

extern rule_t rules[];


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
int init = 0; // random

void parse_options(int* pargc, char** pargv[]);

int main(int argc, char** argv) {
  FILE* fimg;
  Image I;
  char ofname[128];
  int maxbits;
  parse_options(&argc,&argv);
  if (argc > 0) { ifname = argv[0]; }
  if (argc > 1) { ofbasename = argv[1]; }
  fprintf(stdout,"*init=%d\n",init);
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
  fclose(fimg);
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
  srand(seed);

  //
  // initialize state
  //
  if (!is_color(&I)) {
    fprintf(stdout,"grayscale image.\n");
    //
    // grayscale image
    //
    Image S0, S1;
    clone_image(&I,&S0);
    clone_image(&I,&S1);
    const int maxplane = 8;
    if (init == 0) {
      for (int i = 0, li = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++,li++) {
          for (int p = 0; p < maxplane; p++) {
            int t = ((double)rand()/(double)RAND_MAX) > thres;
            S0.pixels[li] |=  (t << p);
          }
        }
      }
    } else {
      copy_image(&I,&S0);
    }
    rule_t rule = rules[ruleno];
    unsigned maxx = 0;
    for (int e = 0; e < epochs; e++) {
        maxx = rule(&I,&S0,&S1);
      snprintf(ofname,128,ofbasename,e);
      if (((movie>0) && (e % movie==0)) || (e==(epochs-1)) ) {
        fprintf(stdout,"-epoch %d output %s maxval %d\n",e,ofname,maxx);
        write_image(&S1,ofname);
      }
      // circular buffer: swap prev and curr states
      Pixel* aux = S1.pixels;
      S1.pixels = S0.pixels;
      S0.pixels = aux;
    }
    fprintf(stdout,"cleanup.\n");
    destroy_image(&S1);
    destroy_image(&S0);
  } else {
    fprintf(stdout,"color image.\n");
    //
    // color image
    //
    Image Ir,Ig,Ib, S0r, S0g, S0b, S1r, S1g, S1b, O;

    init_image(cols,rows,I.type,&Ir);
    init_image(cols,rows,I.type,&Ig);
    init_image(cols,rows,I.type,&Ib);

    init_image(cols,rows,I.type,&S0r);
    init_image(cols,rows,I.type,&S0g);
    init_image(cols,rows,I.type,&S0b);
    erase_image(&S0r);
    erase_image(&S0g);
    erase_image(&S0b);

    init_image(cols,rows,I.type,&S1r);
    init_image(cols,rows,I.type,&S1g);
    init_image(cols,rows,I.type,&S1b);
    erase_image(&S1r);
    erase_image(&S1g);
    erase_image(&S1b);

    init_image(cols,rows,I.type,&O);
    decompose_image(&I,&Ir,&Ig,&Ib);

    const int maxplane = 8;
    if (init == 0) {
      for (int li = 0; li < npixels; li++) {
          for (int p = 0; p < maxplane; p++) {
            int t = ((double)rand()/(double)RAND_MAX) > thres;
            S0r.pixels[li] |=  (t << p);
             t = ((double)rand()/(double)RAND_MAX) > thres;
            S0g.pixels[li] |=  (t << p);
             t = ((double)rand()/(double)RAND_MAX) > thres;
            S0b.pixels[li] |=  (t << p);
        }
      }
    } else {
      copy_image(&Ir,&S0r);
      copy_image(&Ig,&S0g);
      copy_image(&Ib,&S0b);
    }

    rule_t rule = rules[ruleno];
    unsigned maxx = 0;
    for (int e = 0; e < epochs; e++) {
      const int maxr = rule(&Ir,&S0r,&S1r);
      if (maxr > maxx) maxx = maxr;
      const int maxg = rule(&Ig,&S0g,&S1g);
      if (maxg > maxx) maxx = maxg;
      const int maxb = rule(&Ib,&S0b,&S1b);
      if (maxb > maxx) maxx = maxb;
      snprintf(ofname,128,ofbasename,e);
      if (((movie > 0) && (e % movie == 0)) || (e==(epochs-1)) ) {
        fprintf(stdout,"-epoch %d output %s maxval %d\n",e,ofname,maxx);
        compose_image(&S1r,&S1g,&S1b,&O);
        O.maxval = maxx;
        write_image(&O,ofname);
      }
      // circular buffer: swap prev and curr states
      Pixel* aux;
      aux = S1r.pixels; S1r.pixels = S0r.pixels; S0r.pixels = aux;
      aux = S1g.pixels; S1g.pixels = S0g.pixels; S0g.pixels = aux;
      aux = S1b.pixels; S1b.pixels = S0b.pixels; S0b.pixels = aux;
    }
    fprintf(stdout,"cleanup.\n");
    destroy_image(&S0r);
    destroy_image(&S0g);
    destroy_image(&S0b);

    destroy_image(&S1r);
    destroy_image(&S1g);
    destroy_image(&S1b);

    destroy_image(&Ir);
    destroy_image(&Ig);
    destroy_image(&Ib);

    destroy_image(&O);
  }

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
                movie = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                movie = atoi((++argv)[0]);
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
        case 'i':
            if (argv[0][2]) // parameter val is glued , -pval
                init = atoi(argv[0]+2);
            else if (argc-- > 0) // parameter val is next arg
                init = atoi((++argv)[0]);
            printf("init=%d\n",init);
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
  const int maxplane = 8;
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
  const int maxplane =  8;
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
      //const int xu = get_pixel(S0,i-1,j) != 0;
      //const int xd = get_pixel(S0,i+1,j) != 0;
      const int rule = get_pixel(img,i,j);
      const int ri = (((xl << 1) | xc) << 1) | xr;
      const int ruleval = (rule >> ri) & 0x01;
      int prev = get_pixel(S0,i,j);
      int newx = ruleval ? (prev+1) : (prev-1);
      newx  = newx > 255 ? 255: (newx < 0 ? 0: newx);
      set_pixel(S0,i+1,j,newx);
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
// bitplane version of rule 5, Conway
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
  const int maxplane = 8;
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

//
// 8-neighbor bit-planed Conway
//
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
  const int maxplane =  8;
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

Pixel galois1(Image* I, Image* S0, Image* S1) {
  fprintf(stdout,"Galois.");
  const int cols = I->cols;
  const int rows = I->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const gf_t sl = get_pixel_circular(S0,i,j-1);
  const gf_t sr = get_pixel_circular(S0,i,j+1);
  const gf_t su = get_pixel_circular(S0,i-1,j);
  const gf_t sd = get_pixel_circular(S0,i+1,j);
  // rule is xor of 4 neighboring pixels
  const int newx = gf_sum(sd,gf_sum(su,gf_sum(sl,sr)));
//  set_pixel(S0,i,j,newx);
  set_pixel(S1,i,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}


Pixel galois2(Image* I, Image* S0, Image* S1) {
  fprintf(stdout,"Galois II");
  const int cols = I->cols;
  const int rows = I->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
  for (register int j = 0; j < cols; j++) {
  const gf_t sl = get_pixel_circular(S0,i,j-1);
  const gf_t sr = get_pixel_circular(S0,i,j+1);
  const gf_t su = get_pixel_circular(S0,i-1,j);
  const gf_t sd = get_pixel_circular(S0,i+1,j);
  const gf_t sc = get_pixel_circular(S0,i,j);

  // rule is Laplacian operator in Galois Field
  const int newx = gf_sum(gf_mul(4,sc),gf_sum(sd,gf_sum(su,gf_sum(sl,sr))));
  set_pixel(S1,i,j,newx);
  if (newx > maxx) maxx = newx;
  }
  }
  return maxx;
}


Pixel fungus1(Image* I, Image* S0, Image* S1) {
  fprintf(stdout,"Fungus I");
  const int cols = I->cols;
  const int rows = I->rows;
  int maxx = 0;
  for (register int i = 0; i < rows; i++) {
    for (register int j = 0; j < cols; j++) {
      const gf_t sw = get_pixel_circular(S0,i,j-1);
      const gf_t se = get_pixel_circular(S0,i,j+1);
      const gf_t sn = get_pixel_circular(S0,i-1,j);
      const gf_t ss = get_pixel_circular(S0,i+1,j);
      const gf_t snw = get_pixel_circular(S0,i-1,j-1);
      const gf_t sne = get_pixel_circular(S0,i-1,j+1);
      const gf_t ssw = get_pixel_circular(S0,i+1,j-1);
      const gf_t sse = get_pixel_circular(S0,i+1,j+1);
      const gf_t ic = get_pixel_circular(I,i,j);
      //int n = 6;
      //int population = sw + se + sn + ss + (snw + sne + ssw + sse)/2;
      int n = 4.0;
      int population = sw + se + sn + ss;
      // 0 -- die -- t0 -- stay -- t1 -- grow -- t2 -- die
      const int t2 = ic*n*0.6;
      const int t1 = ic*n*0.4;
      const int t0 = ic*n*0.3;
      // rule is Laplacian operator in Galois Field
      int newx = get_pixel_circular(S0,i,j);
      if (newx > 0 && population > t2) newx--;
      else if (newx > 0 && population < t0) newx--;
      else if (newx < 255 && population > t1) newx++;
      set_pixel(S1,i,j,newx);
      if (newx > maxx) maxx = newx;
    }
  }
  return maxx;
}

rule_t rules[] = {rule0, rule1, rule2, rule3, rule4, rule5, rule6, rule7, rule8, galois1, galois2,fungus1};
