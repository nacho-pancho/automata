#include "context.h"

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
