#include <fstd_map.h>
#include <string.h>
#include <unity.h>

typedef struct elem_t {
  uint32_t a;
  float b;
} elem_t;

void test_map_empty() {
  fstd_map_t map;
  fstd_map_init(&map, 7, elem_t);

  fstd_map_destroy(&map);
}

void test_map_basic() {
  fstd_map_t map;
  fstd_map_init(&map, 7, elem_t);

  elem_t *elem = fstd_map_get(&map, "Hello");
  TEST_ASSERT_NULL(elem);

  elem_t to_add = {.a = 2, .b = 3.14f};

  elem = fstd_map_set(&map, "Hello", &to_add);
  TEST_ASSERT_EQUAL(map.filled, 1);
  TEST_ASSERT_NOT_NULL(elem);
  TEST_ASSERT(memcmp(elem, &to_add, sizeof(elem_t)) == 0);

  elem = fstd_map_get(&map, "Hello");
  TEST_ASSERT_NOT_NULL(elem);
  TEST_ASSERT(memcmp(elem, &to_add, sizeof(elem_t)) == 0);

  TEST_ASSERT_EQUAL_PTR(elem, fstd_map_remove(&map, "Hello"));
  TEST_ASSERT_EQUAL(map.filled, 0);

  fstd_map_destroy(&map);
}

void test_map_double_set() {
  fstd_map_t map;
  fstd_map_init(&map, 7, elem_t);

  elem_t *elem = fstd_map_get(&map, "Hello");

  TEST_ASSERT_NULL(elem);

  elem_t to_add = {.a = 2, .b = 3.14f};

  elem_t *elem1 = fstd_map_set(&map, "Hello", &to_add);
  TEST_ASSERT_NOT_NULL(elem1);
  TEST_ASSERT(memcmp(elem1, &to_add, sizeof(elem_t)) == 0);

  elem_t *elem2 = fstd_map_set(&map, "Hello", &to_add);
  TEST_ASSERT_NOT_NULL(elem2);
  TEST_ASSERT(memcmp(elem2, &to_add, sizeof(elem_t)) == 0);

  TEST_ASSERT_EQUAL_PTR(elem1, elem2);

  fstd_map_destroy(&map);
}

void test_map_remove_nonexistant() {
  fstd_map_t map;
  fstd_map_init(&map, 7, elem_t);

  elem_t *elem = fstd_map_remove(&map, "Hello");
  TEST_ASSERT_EQUAL_PTR(NULL, elem);

  fstd_map_destroy(&map);
}

void test_map_capacity() {
  fstd_map_t map;
  fstd_map_init(&map, 2, int);

  int *elem1 = fstd_map_set(&map, "Hello", &(int){1});
  TEST_ASSERT_NOT_NULL(elem1);
  TEST_ASSERT_EQUAL(*elem1, 1);

  int *elem2 = fstd_map_set(&map, "World", &(int){2});
  TEST_ASSERT_NOT_NULL(elem2);
  TEST_ASSERT_EQUAL(*elem2, 2);

  int *elem3 = fstd_map_set(&map, "Hey", &(int){3});
  TEST_ASSERT_NULL(elem3);

  elem3 = fstd_map_get(&map, "Hey");
  TEST_ASSERT_NULL(elem3);

  fstd_map_destroy(&map);
}

void test_map_collision() {
  fstd_map_t map;
  fstd_map_init(&map, 3, int);

  size_t hash1 = fstd__djb_hash("Hey") % map.capacity;
  size_t hash2 = fstd__djb_hash("World") % map.capacity;
  TEST_ASSERT_EQUAL_INT64(hash1, hash2);

  int *elem1 = fstd_map_set(&map, "Hey", &(int){1});
  TEST_ASSERT_NOT_NULL(elem1);

  int *elem2 = fstd_map_set(&map, "World", &(int){2});
  TEST_ASSERT_NOT_NULL(elem2);

  int *elemi1 = fstd_map_get_by_index(&map, hash1, NULL);
  int *elemi2 = fstd_map_get_by_index(&map, hash2, NULL);
  TEST_ASSERT_EQUAL_PTR(elemi1, elem1);
  TEST_ASSERT_EQUAL_PTR(elemi2, elem1);

  int *elemi3 = fstd_map_get_by_index(&map, hash2 + 1, NULL);
  TEST_ASSERT_EQUAL_PTR(elemi3, elem2);

  fstd_map_destroy(&map);
}

void test_map_read_key_from_index() {
  fstd_map_t map;
  fstd_map_init(&map, 1, int);

  fstd_map_set(&map, "Hey", &(int){1});

  char *key;
  TEST_ASSERT(fstd_map_get_by_index(&map, 0, &key));
  TEST_ASSERT_EQUAL_STRING("Hey", key);

  fstd_map_destroy(&map);
}

void test_map_read_key_from_value() {
  fstd_map_t map;
  fstd_map_init(&map, 1, int);

  int *val = fstd_map_set(&map, "Hey", &(int){1});

  char *key = fstd_map_get_key(&map, val);
  TEST_ASSERT_EQUAL_STRING("Hey", key);

  fstd_map_destroy(&map);
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_map_empty);
  RUN_TEST(test_map_basic);
  RUN_TEST(test_map_double_set);
  RUN_TEST(test_map_remove_nonexistant);
  RUN_TEST(test_map_capacity);
  RUN_TEST(test_map_collision);
  RUN_TEST(test_map_read_key_from_index);
  RUN_TEST(test_map_read_key_from_value);

  return UNITY_END();
}
