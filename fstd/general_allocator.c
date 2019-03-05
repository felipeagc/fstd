#include "general_allocator.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

static inline void general_header_init(
    fstd_general_alloc_header_t *header,
    size_t size,
    fstd_general_alloc_header_t *prev,
    fstd_general_alloc_header_t *next) {
  header->addr = ((uint8_t *)header) + sizeof(fstd_general_alloc_header_t);
  header->prev = prev;
  header->next = next;
  header->size = size;
  header->used = false;
}

static inline void
general_header_merge_if_necessary(fstd_general_alloc_header_t *header) {
  assert(header != NULL);
  assert(!header->used);

  if (header->prev != NULL && !header->prev->used) {
    header->prev->next = header->next;
    header->prev->size += header->size + sizeof(fstd_general_alloc_header_t);
    header = header->prev;
  }

  if (header->next != NULL && !header->next->used) {
    header->size += header->next->size + sizeof(fstd_general_alloc_header_t);
    header->next = header->next->next;
  }
}

static inline void
general_block_init(fstd_general_block_t *block, size_t block_size) {
  assert(block_size > sizeof(fstd_general_alloc_header_t));
  block->storage = (uint8_t *)malloc(block_size);
  block->first_header = (fstd_general_alloc_header_t *)block->storage;
  general_header_init(
      block->first_header,
      block_size - sizeof(fstd_general_alloc_header_t),
      NULL,
      NULL);
  block->next = NULL;
  block->prev = NULL;
}

static inline void general_block_destroy(fstd_general_block_t *block) {
  if (block == NULL) {
    return;
  }

  general_block_destroy(block->next);
  if (block->next != NULL) {
    free(block->next);
  }

  free(block->storage);
}

void fstd_general_allocator_init(
    fstd_general_allocator_t *allocator, size_t block_size) {

  allocator->block_size = block_size;

  general_block_init(&allocator->base_block, block_size);

  allocator->last_block = &allocator->base_block;

  fstd_mutex_init(&allocator->mutex);
}

void fstd_general_allocator_destroy(fstd_general_allocator_t *allocator) {
  fstd_mutex_lock(&allocator->mutex);
  general_block_destroy(&allocator->base_block);
  fstd_mutex_unlock(&allocator->mutex);
  fstd_mutex_destroy(&allocator->mutex);
}

void *
fstd_general_allocator_alloc(fstd_general_allocator_t *allocator, uint32_t size) {
  fstd_mutex_lock(&allocator->mutex);

  if (size >= allocator->block_size + sizeof(fstd_general_alloc_header_t)) {
    fstd_mutex_unlock(&allocator->mutex);
    return NULL;
  }

  fstd_general_alloc_header_t *best_header = NULL;
  fstd_general_block_t *block = allocator->last_block;
  bool can_insert_new_header = false;

  uint32_t padding = 0;
  if (size % 8 != 0) {
    padding = 8 - (size % 8);
  }

  while (block != NULL) {
    fstd_general_alloc_header_t *header = block->first_header;
    while (header != NULL) {
      if (!header->used && header->size >= size) {
        best_header = header;

        can_insert_new_header =
            header->size >= size + padding + sizeof(fstd_general_alloc_header_t);
        break;
      }

      header = header->next;
    }

    if (best_header != NULL) {
      break;
    }

    block = block->prev;
  }

  if (best_header == NULL) {
    fstd_general_block_t *new_block =
        (fstd_general_block_t *)malloc(sizeof(fstd_general_block_t));
    general_block_init(new_block, allocator->block_size);

    new_block->prev = allocator->last_block;

    allocator->last_block->next = new_block;
    allocator->last_block = new_block;

    fstd_mutex_unlock(&allocator->mutex);
    return fstd_general_allocator_alloc(allocator, size);
  }

  if (can_insert_new_header) {
    // @NOTE: insert new header after the allocation
    uint32_t old_size = best_header->size;
    best_header->size = size + padding;
    fstd_general_alloc_header_t *new_header =
      (fstd_general_alloc_header_t *) ((uint8_t *)best_header +
                                     sizeof(fstd_general_alloc_header_t) +
                                     best_header->size);
    general_header_init(
        new_header,
        old_size - best_header->size - sizeof(fstd_general_alloc_header_t),
        best_header,
        best_header->next);
    best_header->next = new_header;
  }

  best_header->used = true;

  fstd_mutex_unlock(&allocator->mutex);
  return best_header->addr;
}

void fstd_general_allocator_free(fstd_general_allocator_t *allocator, void *ptr) {
  fstd_general_alloc_header_t *header =
      (fstd_general_alloc_header_t
           *)(((uint8_t *)ptr) - sizeof(fstd_general_alloc_header_t));
  assert(header->addr == ptr);
  header->used = false;
  general_header_merge_if_necessary(header);
}
