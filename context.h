#ifndef CONTEXT_H
#define CONTEXT_H
#include "tpl.h"
/**
 * Context realization
 */
typedef struct {
    val_t* values;
    unsigned int k;
} Context;

/**
 * Strategy for mapping contexts
 */
typedef void (*ContextMapper)(const Context* orig_ctx, Context* mapped_ctx);

#endif
