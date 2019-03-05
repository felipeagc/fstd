#include <fstd/general_allocator.h>
#include <unity.h>

uint32_t header_count(fstd_general_block_t *block) {
  uint32_t headers = 0;

  fstd_general_alloc_header_t *header = block->first_header;
  while (header != NULL) {
    headers++;
    header = header->next;
  }

  return headers;
}

uint32_t block_count(fstd_general_allocator_t *allocator) {
  uint32_t blocks = 0;

  fstd_general_block_t *block = allocator->last_block;
  while (block != NULL) {
    blocks++;
    block = block->prev;
  }

  return blocks;
}

void test_general_allocator_create_destroy() {
  fstd_general_allocator_t allocator;
  fstd_general_allocator_init(&allocator, 120);

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 1);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  uint32_t *alloc1 = fstd_GENERAL_ALLOC(&allocator, uint32_t);
  *alloc1 = 32;

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 2);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  TEST_ASSERT_EQUAL_PTR(
      allocator.last_block->first_header + sizeof(fstd_general_alloc_header_t),
      allocator.last_block->first_header->addr);

  fstd_general_allocator_free(&allocator, alloc1);
  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 1);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  uint32_t *alloc2 = fstd_GENERAL_ALLOC(&allocator, uint32_t);
  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 2);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  uint32_t *alloc3 = fstd_GENERAL_ALLOC(&allocator, uint32_t);
  *alloc3 = 64;
  TEST_ASSERT_EQUAL_UINT32(*alloc3, 64);

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 3);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  fstd_general_allocator_free(&allocator, alloc3);

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 2);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  TEST_ASSERT_EQUAL_UINT32((uintptr_t)alloc1, (uintptr_t)alloc2);
  TEST_ASSERT_EQUAL_UINT32(*alloc2, 32);

  alloc3 = fstd_GENERAL_ALLOC(&allocator, uint32_t);

  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 3);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);

  uint32_t *alloc4 = fstd_GENERAL_ALLOC(&allocator, uint32_t);
  *alloc4 = 36;
  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 3);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 1);
  TEST_ASSERT_EQUAL_UINT32(*alloc4, 36);

  uint32_t *alloc5 = fstd_GENERAL_ALLOC(&allocator, uint32_t);
  *alloc5 = 45;
  TEST_ASSERT_EQUAL_UINT32(header_count(allocator.last_block), 2);
  TEST_ASSERT_EQUAL_UINT32(block_count(&allocator), 2);
  TEST_ASSERT_EQUAL_UINT32(*alloc5, 45);

  typedef uint32_t bigarray[3000];
  bigarray *alloc6 = fstd_GENERAL_ALLOC(&allocator, bigarray);
  TEST_ASSERT_EQUAL_PTR(alloc6, NULL);

  fstd_general_allocator_destroy(&allocator);
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_general_allocator_create_destroy);

  return UNITY_END();
}