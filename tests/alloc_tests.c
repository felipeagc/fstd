#include <assert.h>
#include <fstd_alloc.h>
#include <unity.h>

typedef FSTD__ALLOC_ALIGNAS(16) struct vec4_t { float v[4]; } vec4_t;
typedef FSTD__ALLOC_ALIGNAS(16) struct mat4_t { float v[16]; } mat4_t;

uint32_t header_count(fstd_alloc_block_t *block) {
  uint32_t headers = 0;

  fstd_alloc_header_t *header = block->first_header;
  while (header != NULL) {
    headers++;
    header = header->next;
  }

  return headers;
}

uint32_t block_count(fstd_allocator_t *allocator) {
  uint32_t blocks = 0;

  fstd_alloc_block_t *block = allocator->last_block;
  while (block != NULL) {
    blocks++;
    block = block->prev;
  }

  return blocks;
}

#define BLOCK_SIZE(alloc_size, count)                                          \
  ((sizeof(fstd_alloc_header_t) + (alloc_size)) * count)

void test_create_destroy() {
  fstd_allocator_t allocator;
  fstd_allocator_init(&allocator, 120);

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 1);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  fstd_allocator_destroy(&allocator);
}

void test_alloc_simple() {
  fstd_allocator_t allocator;
  fstd_allocator_init(&allocator, 120);

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 1);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  uint32_t *alloc1 = fstd_alloc(&allocator, sizeof(uint32_t));
  TEST_ASSERT(alloc1 != NULL);
  *alloc1 = 32;

  fstd_allocator_destroy(&allocator);
}

void test_alloc_aligned() {
  fstd_allocator_t allocator;
  fstd_allocator_init(&allocator, 1 << 14);

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 1);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  {
    vec4_t *alloc = fstd_alloc(&allocator, sizeof(vec4_t));
    TEST_ASSERT(alloc != NULL);
    *alloc = (vec4_t){{0.0, 0.0, 0.0, 0.0}};
  }

  {
    vec4_t *alloc = fstd_alloc(&allocator, sizeof(vec4_t));
    TEST_ASSERT(alloc != NULL);
    *alloc = (vec4_t){{0.0, 0.0, 0.0, 0.0}};
  }

  {
    char *alloc = fstd_alloc(&allocator, sizeof(char));
    TEST_ASSERT(alloc != NULL);
  }

  {
    vec4_t *alloc = fstd_alloc(&allocator, sizeof(vec4_t));
    TEST_ASSERT(alloc != NULL);
    *alloc = (vec4_t){{0.0, 0.0, 0.0, 0.0}};
  }

  {
    char *alloc = fstd_alloc(&allocator, sizeof(char));
    TEST_ASSERT(alloc != NULL);
  }

  {
    char *alloc = fstd_alloc(&allocator, sizeof(char));
    TEST_ASSERT(alloc != NULL);
  }

  {
    mat4_t *alloc = fstd_alloc(&allocator, sizeof(mat4_t));
    TEST_ASSERT(alloc != NULL);
    *alloc = (mat4_t){0};
  }

  {
    vec4_t *alloc = fstd_alloc(&allocator, sizeof(vec4_t));
    TEST_ASSERT(alloc != NULL);
    *alloc = (vec4_t){{0.0, 0.0, 0.0, 0.0}};
  }

  fstd_allocator_destroy(&allocator);
}

void test_alloc_multi_block() {
  fstd_allocator_t allocator;

  uint32_t per_block = 3;
  size_t alloc_size = FSTD__ALLOC_ALIGNMENT;

  fstd_allocator_init(
      &allocator, (sizeof(fstd_alloc_header_t) + alloc_size) * per_block);

  for (uint32_t i = 0; i < per_block * 20; i++) {
    uint64_t *alloc = fstd_alloc(&allocator, alloc_size);
    TEST_ASSERT(alloc != NULL);
    TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), (i / per_block) + 1);
  }

  fstd_allocator_destroy(&allocator);
}

void test_alloc_too_big() {
  fstd_allocator_t allocator;

  fstd_allocator_init(&allocator, 160);

  {
    uint64_t *alloc = fstd_alloc(&allocator, 160);
    TEST_ASSERT(alloc == NULL);
  }

  {
    uint64_t *alloc = fstd_alloc(&allocator, 10000);
    TEST_ASSERT(alloc == NULL);
  }

  {
    uint64_t *alloc = fstd_alloc(
        &allocator, allocator.block_size - sizeof(fstd_alloc_header_t));
    TEST_ASSERT(alloc != NULL);
  }

  fstd_allocator_destroy(&allocator);
}

void test_alloc_free() {
  fstd_allocator_t allocator;

  fstd_allocator_init(&allocator, 160);

  {
    void *alloc1 = fstd_alloc(
        &allocator, allocator.block_size - sizeof(fstd_alloc_header_t));
    TEST_ASSERT(alloc1 != NULL);
    fstd_free(&allocator, alloc1);

    void *alloc2 = fstd_alloc(
        &allocator, allocator.block_size - sizeof(fstd_alloc_header_t));
    TEST_ASSERT(alloc2 != NULL);

    TEST_ASSERT(alloc1 == alloc2);
  }

  fstd_allocator_destroy(&allocator);
}

void test_alloc_realloc_grow() {
  fstd_allocator_t allocator;

  fstd_allocator_init(&allocator, 160);

  {
    uint32_t *alloc1 = fstd_alloc(&allocator, sizeof(uint32_t));
    TEST_ASSERT(alloc1 != NULL);
    TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

    uint32_t *alloc2 = fstd_realloc(&allocator, alloc1, sizeof(uint32_t) * 4);
    TEST_ASSERT(alloc2 != NULL);
    TEST_ASSERT(alloc2 == alloc1);
    TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

    uint32_t *alloc3 = fstd_realloc(
        &allocator, alloc2, allocator.block_size - sizeof(fstd_alloc_header_t));
    TEST_ASSERT(alloc3 != NULL);
    TEST_ASSERT(alloc3 == alloc2);
    TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);
  }

  fstd_allocator_destroy(&allocator);
}

void test_alloc_realloc_fragmented() {
  fstd_allocator_t allocator;

  fstd_allocator_init(&allocator, 160);

  {
    uint32_t *alloc1 = fstd_alloc(&allocator, sizeof(uint32_t));
    TEST_ASSERT(alloc1 != NULL);
    *alloc1 = 3;

    void *alloc2 = fstd_alloc(&allocator, 4);
    TEST_ASSERT(alloc2 != NULL);

    uint32_t *alloc3 =
        fstd_realloc(&allocator, alloc1, FSTD__ALLOC_ALIGNMENT + 1);
    TEST_ASSERT(alloc3 != NULL);
    TEST_ASSERT(alloc3 != alloc1);
    TEST_ASSERT(*alloc3 == 3);
  }

  fstd_allocator_destroy(&allocator);
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_create_destroy);
  RUN_TEST(test_alloc_simple);
  RUN_TEST(test_alloc_aligned);
  RUN_TEST(test_alloc_multi_block);
  RUN_TEST(test_alloc_too_big);
  RUN_TEST(test_alloc_free);
  RUN_TEST(test_alloc_realloc_grow);
  RUN_TEST(test_alloc_realloc_fragmented);

  return UNITY_END();
}
