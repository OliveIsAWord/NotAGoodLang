#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define VECTOR_INIT_CAP 8

#define vector(type) type *

#define _internal_veccap(vec, size)         \
    do {                                    \
        if (vec) {                          \
            ((size_t *)(vec))[-1] = (size); \
        }                                   \
    } while (0)

#define _internal_vecsiz(vec, size)         \
    do {                                    \
        if (vec) {                          \
            ((size_t *)(vec))[-2] = (size); \
        }                                   \
    } while (0)

#define vector_get_capacity(vec) \
    ((vec) ? ((size_t *)(vec))[-1] : (size_t)0)

#define vector_get_size(vec) \
    ((vec) ? ((size_t *)(vec))[-2] : (size_t)0)

#define vector_is_empty(vec) \
    (vector_get_size(vec) == 0)

#define vector_grow(vec, count)                                               \
    do {                                                                      \
        const size_t cv_sz = (count) * sizeof(*(vec)) + (sizeof(size_t) * 2); \
        if (!(vec)) {                                                         \
            size_t *cv_p = malloc(cv_sz);                                     \
            assert(cv_p);                                                     \
            (vec) = (void *)(&cv_p[2]);                                       \
            _internal_veccap((vec), (count));                                 \
            _internal_vecsiz((vec), 0);                                       \
        } else {                                                              \
            size_t *cv_p1 = &((size_t *)(vec))[-2];                           \
            size_t *cv_p2 = realloc(cv_p1, (cv_sz));                          \
            assert(cv_p2);                                                    \
            (vec) = (void *)(&cv_p2[2]);                                      \
            _internal_veccap((vec), (count));                                 \
        }                                                                     \
    } while (0)

#define vector_pop(vec)                                    \
    do {                                                   \
        _internal_vecsiz((vec), vector_get_size(vec) - 1); \
    } while (0)

#define vector_erase(vec, i)                                   \
    do {                                                       \
        if (vec) {                                             \
            const size_t cv_sz = vector_get_size(vec);         \
            if ((i) < cv_sz) {                                 \
                _internal_vecsiz((vec), cv_sz - 1);            \
                size_t cv_x;                                   \
                for (cv_x = (i); cv_x < (cv_sz - 1); ++cv_x) { \
                    (vec)[cv_x] = (vec)[cv_x + 1];             \
                }                                              \
            }                                                  \
        }                                                      \
    } while (0)

#define vector_free(vec)                         \
    do {                                         \
        if (vec) {                               \
            size_t *p1 = &((size_t *)(vec))[-2]; \
            free(p1);                            \
        }                                        \
    } while (0)

#define vector_begin(vec) \
    (vec)

#define vector_end(vec) \
    ((vec) ? &((vec)[vector_get_size(vec)]) : NULL)

#define vector_push(vec, value)                                              \
    do {                                                                     \
        size_t cv_cap = vector_get_capacity(vec);                            \
        if (cv_cap <= vector_get_size(vec)) {                                \
            size_t new_cap = cv_cap ? cv_cap + cv_cap / 2 : VECTOR_INIT_CAP; \
            vector_grow((vec), new_cap);                                     \
        }                                                                    \
        (vec)[vector_get_size(vec)] = (value);                               \
        _internal_vecsiz((vec), vector_get_size(vec) + 1);                   \
    } while (0)

/// Safety: `vec` must be non-null
#define _internal_true_ptr(vec) \
    (&((size_t *)(vec))[-2])
/// Safety: `vec` must be non-null
#define _internal_true_cap(vec) \
    (vector_get_capacity(vec) * sizeof(*(vec)) - sizeof(size_t) * 2)

#define vector_shrink_to_fit(vec)                                          \
    do {                                                                   \
        size_t cv_cap = vector_get_capacity(vec);                          \
        size_t cv_siz = vector_get_size(vec);                              \
        if (cv_cap > cv_siz) {                                             \
            size_t fit_cap = cv_siz * sizeof(*(vec)) + sizeof(size_t) * 2; \
            void *ptr = realloc(_internal_true_ptr(vec), fit_cap);         \
            (vec) = ptr + sizeof(size_t) * 2;                              \
            _internal_veccap((vec), cv_siz);                               \
        }                                                                  \
    } while (0)

#define vector_get(vec, index)                \
    do {                                      \
        if ((index) < vector_get_size(vec)) { \
            return &(vec)[(index)];           \
        } else {                              \
            return NULL;                      \
        }                                     \
    } while (0)
