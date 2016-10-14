#include "ascmat.h"
#include <stdlib.h>
#include "img.h"

#define LINEARIZE 0

/*---------------------------------------------------------------------------------------*/

int get_pixel(const Image* pimg, int i, int j) {
    return (i < 0) || (i >= pimg->rows) || (j < 0) || (j >= pimg->cols) ? 0 : pimg->pixels[i*pimg->cols+j];
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

/*---------------------------------------------------------------------------------------*/

void linearize_template(const Template* pt, int nrows, int ncols, const LinearTemplate* plt) {
    const int k = pt->k;
    int j;
    for (j=0; j<k; ++j) {
        plt->li[j] = pt->is[j]*ncols+pt->js[j];
    }
}

/*---------------------------------------------------------------------------------------*/

Template* ini_template(Template* pt, int k) {
    if (pt == NULL) {
        pt = (Template*) malloc(sizeof(Template));
    }
    pt->k = k;
    pt->is = (index_t*) malloc(sizeof(index_t)*k);
    pt->js = (index_t*) malloc(sizeof(index_t)*k);
    return pt;
}

void destroy_template(Template* pt) {
    if (pt == NULL) return;
    pt->k = 0;
    free(pt->is);
    free(pt->js);
}

/*---------------------------------------------------------------------------------------*/

Template* generate_random_template(int max_l1_radius, int k, Template* pt) {
    int r;
    if (pt == NULL) {
        pt = ini_template(NULL,k);
    }
    for (r=0 ; r<k ; r++) {
        // sample i and j
        int i = (int)(2.*(double)max_l1_radius*(double)rand()/(double)RAND_MAX - (double)max_l1_radius+ 0.5);
        int j = (int)(2.*(double)max_l1_radius*(double)rand()/(double)RAND_MAX - (double)max_l1_radius+ 0.5);
        if (i*j == 0) {
            r--;
            continue;
        }
        // see if not already there
        int r2 ;
        for (r2 = 0; r2 < r; r2++) {
            if ((i==pt->is[r2]) && (j==pt->js[r2])) {
                break;
            }
        }
        if (r2 < r) {
            r--;
            continue;
        }
        pt->is[r] = i;
        pt->js[r] = j;
    }
    return pt;
}

/*---------------------------------------------------------------------------------------*/

void read_template(const char* fname, Template* ptpl) {
    double* tpldat  = 0;
    unsigned nrows = 0;
    unsigned ncols = 0;
    int i;
    read_ascii_matrix(fname,&nrows,&ncols,&tpldat);
    printf("TEMPLATE (k=%d):\n",ncols);
    print_ascii_matrix(nrows,ncols,tpldat);
    ini_template(ptpl,ncols);
    for (i = 0; i < ncols; i++) {
        ptpl->is[i] = tpldat[i];
        ptpl->js[i] = tpldat[ncols+i];
    }
    free(tpldat);
}

/*---------------------------------------------------------------------------------------*/

void read_template_multi(const char* fname, Template** ptpls, int* pntpl) {
    double* tpldat  = 0;
    unsigned nrows = 0;
    unsigned ncols = 0;
    Template* tpls;
    int i,t;
    read_ascii_matrix(fname,&nrows,&ncols,&tpldat);
    //print_ascii_matrix(nrows,ncols,tpldat);
    int ntemplates = nrows / 2;
    tpls = (Template*) malloc(sizeof(Template)*ntemplates);
    for (t = 0; t < ntemplates; t++) {
        printf("TEMPLATE %d (k=%d):\n",t,ncols);
        ini_template(&tpls[t],ncols);
        for (i = 0; i < ncols; i++) {
            const int ii = tpls[t].is[i] = tpldat[2*t*ncols+i];
            const int ji = tpls[t].js[i] = tpldat[(2*t+1)*ncols+i];
	    if ((ii==0) && (ji==0)) {
                tpls[t].k = i;
                break;
            }
        }
        print_template(&tpls[t]);
    }
    free(tpldat);
    *ptpls = tpls;
    *pntpl = ntemplates;
}

/*---------------------------------------------------------------------------------------*/
void print_template(const Template* ptpl) {
    int min_i = 10000,min_j = 10000, max_i=-10000, max_j=-10000;
    int k,r,l;
    for (k = 0; k < ptpl->k; k++) {
        if (ptpl->is[k] < min_i) min_i = ptpl->is[k];
        if (ptpl->is[k] > max_i) max_i = ptpl->is[k];
        if (ptpl->js[k] < min_j) min_j = ptpl->js[k];
        if (ptpl->js[k] > max_j) max_j = ptpl->js[k];
    }
    int a = max_i - min_i;
    int b = max_j - min_j;
    printf("    |");
    for (r = 0; r <= b; r++) {
        printf(" %2d ",r+min_j);
    }
    printf("\n----+");
    for (r = 0; r <= b; r++) {
        printf("----");
    }
    putchar('\n');
    for (k = 0; k <= a; k++) {
        printf(" %2d |",min_i+k);
        for (r = 0; r <= b; r++) {
            for (l=0; l < ptpl->k; l++) {
                if ((ptpl->is[l] == (min_i+k)) && (ptpl->js[l] == (min_j+r))) {
                    printf(" %2d ",l);
                    break;
                }
            }
            if (l == ptpl->k)
                printf("    ");
        }
        putchar('\n');
    }

}

/*---------------------------------------------------------------------------------------*/

void get_context(const Image* pimg, const Template* ptpl, int i, int j, ContextMapper mapper, Context* pctx) {
    const int k = ptpl->k;
    register int r;
    for (r = 0; r < k; ++r) {
        pctx->values[r] = get_pixel(pimg, i+ptpl->is[r], j+ptpl->js[r]);
    }
    if (mapper!=NULL) mapper(pctx,pctx);
}

void print_context(const Context* pctx) {
    int j;
    printf("[ ");
    for (j=0; j < pctx->k; j++) {
       printf("%03d ",pctx->values[j]);
    }
    printf("]\n");
}

/*---------------------------------------------------------------------------------------*/

void get_linear_context(const Image* pimg, const LinearTemplate* ptpl, int i, int j, ContextMapper mapper, Context* pctx) {
    const int k = ptpl->k;
    const index_t* const lis = ptpl->li;
    const index_t offset = i*pimg->cols+j;
    val_t* pcv = pctx->values;
    register int r;
    for (r = 0; r < k; ++r) {
        pcv[r] = get_linear_pixel(pimg, offset+lis[r]);
    }
    if (mapper!=NULL) mapper(pctx,pctx);
}
