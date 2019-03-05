#ifndef FSTD_GENERAL_ALLOCATOR_H
#define FSTD_GENERAL_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"
#include <stddef.h>
#include <stdint.h>

#define fstd_GENERAL_ALLOC(allocator, type)                                      \
  (type *)fstd_general_allocator_alloc(allocator, sizeof(type))

typedef struct fstd_general_alloc_header_t {
  /* 0x00 */ uint32_t size;
  /* 0x04 */ uint32_t used;
  /* 0x0C */ void *addr;
  /* 0x10 */ struct fstd_general_alloc_header_t *prev;
  /* 0x18 */ struct fstd_general_alloc_header_t *next;
} fstd_general_alloc_header_t;

typedef struct fstd_general_alloc_block_t {
  uint8_t *storage;
  fstd_general_alloc_header_t *first_header;
  struct fstd_general_alloc_block_t *next;
  struct fstd_general_alloc_block_t *prev;
} fstd_general_block_t;

typedef struct fstd_general_allocator_t {
  fstd_general_block_t base_block;
  fstd_general_block_t *last_block;
  size_t block_size;
  fstd_mutex_t mutex;
} fstd_general_allocator_t;

void fstd_general_allocator_init(
    fstd_general_allocator_t *allocator, size_t block_size);

void fstd_general_allocator_destroy(fstd_general_allocator_t *allocator);

void *
fstd_general_allocator_alloc(fstd_general_allocator_t *allocator, uint32_t size);

void fstd_general_allocator_free(fstd_general_allocator_t *allocator, void *ptr);

#ifdef __cplusplus
}
#endif

#endif
