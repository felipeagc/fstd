#include <fstd_bitset.h>
#include <unity.h>

void test_bitset_size() {
  TEST_ASSERT_EQUAL(sizeof(FSTD_BITSET(1)), 1);
  TEST_ASSERT_EQUAL(sizeof(FSTD_BITSET(8)), 1);
  TEST_ASSERT_EQUAL(sizeof(FSTD_BITSET(9)), 2);
  TEST_ASSERT_EQUAL(sizeof(FSTD_BITSET(15)), 2);
  TEST_ASSERT_EQUAL(sizeof(FSTD_BITSET(16)), 2);
  TEST_ASSERT_EQUAL(sizeof(FSTD_BITSET(17)), 3);
}

void test_bitset_set() {
  FSTD_BITSET(47) bitset;
  fstd_bitset_reset(&bitset, 47);
  TEST_ASSERT_EQUAL(fstd_bitset_at(&bitset, 35), false);
  fstd_bitset_set(&bitset, 35, true);
  fstd_bitset_set(&bitset, 35, true);
  TEST_ASSERT_EQUAL(fstd_bitset_at(&bitset, 35), true);
  fstd_bitset_set(&bitset, 35, false);
  TEST_ASSERT_EQUAL(fstd_bitset_at(&bitset, 35), false);
  fstd_bitset_set(&bitset, 35, true);
  TEST_ASSERT_EQUAL(fstd_bitset_at(&bitset, 35), true);
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_bitset_size);
  RUN_TEST(test_bitset_set);

  return UNITY_END();
}
