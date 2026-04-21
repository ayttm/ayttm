/*
 * tests/test_value_pair.c — unit tests for src/value_pair.c
 *
 * Tests: value_pair_add, value_pair_get_value, value_pair_remove,
 *        value_pair_update, value_pair_free
 */

#include <glib.h>
#include "value_pair.h"

/* Helper: build a list with n (key, value) pairs */
static GList *make_list(const char *k1, const char *v1)
{
	return value_pair_add(NULL, k1, v1);
}

/* ---------- value_pair_get_value ---------------------------------------- */

static void test_get_null_list(void)
{
	g_assert_null(value_pair_get_value(NULL, "key"));
}

static void test_get_missing_key(void)
{
	GList *list = make_list("foo", "bar");
	g_assert_null(value_pair_get_value(list, "baz"));
	value_pair_free(list);
}

static void test_get_found(void)
{
	GList *list = make_list("name", "alice");
	char *val = value_pair_get_value(list, "name");
	g_assert_cmpstr(val, ==, "alice");
	g_free(val);
	value_pair_free(list);
}

static void test_get_case_insensitive(void)
{
	GList *list = make_list("Name", "alice");
	char *val = value_pair_get_value(list, "name");
	g_assert_cmpstr(val, ==, "alice");
	g_free(val);
	val = value_pair_get_value(list, "NAME");
	g_assert_cmpstr(val, ==, "alice");
	g_free(val);
	value_pair_free(list);
}

static void test_get_returns_copy(void)
{
	/* Modifying the returned string must not affect the stored value. */
	GList *list = make_list("x", "original");
	char *val = value_pair_get_value(list, "x");
	val[0] = 'X';	/* mutate the copy */
	char *val2 = value_pair_get_value(list, "x");
	g_assert_cmpstr(val2, ==, "original");
	g_free(val);
	g_free(val2);
	value_pair_free(list);
}

/* ---------- value_pair_add ---------------------------------------------- */

static void test_add_to_null(void)
{
	GList *list = value_pair_add(NULL, "k", "v");
	g_assert_nonnull(list);
	g_assert_cmpint(g_list_length(list), ==, 1);
	char *v = value_pair_get_value(list, "k");
	g_assert_cmpstr(v, ==, "v");
	g_free(v);
	value_pair_free(list);
}

static void test_add_new_key(void)
{
	GList *list = make_list("a", "1");
	list = value_pair_add(list, "b", "2");
	g_assert_cmpint(g_list_length(list), ==, 2);
	char *v = value_pair_get_value(list, "b");
	g_assert_cmpstr(v, ==, "2");
	g_free(v);
	value_pair_free(list);
}

static void test_add_replaces_existing(void)
{
	GList *list = make_list("key", "old");
	list = value_pair_add(list, "key", "new");
	/* Only one entry with this key should remain */
	g_assert_cmpint(g_list_length(list), ==, 1);
	char *v = value_pair_get_value(list, "key");
	g_assert_cmpstr(v, ==, "new");
	g_free(v);
	value_pair_free(list);
}

static void test_add_replaces_case_insensitive(void)
{
	GList *list = make_list("KEY", "old");
	list = value_pair_add(list, "key", "new");
	g_assert_cmpint(g_list_length(list), ==, 1);
	char *v = value_pair_get_value(list, "KEY");
	g_assert_cmpstr(v, ==, "new");
	g_free(v);
	value_pair_free(list);
}

/* ---------- value_pair_remove ------------------------------------------- */

static void test_remove_null_list(void)
{
	GList *list = value_pair_remove(NULL, "x");
	g_assert_null(list);
}

static void test_remove_missing_key(void)
{
	GList *list = make_list("a", "1");
	list = value_pair_remove(list, "z");
	g_assert_cmpint(g_list_length(list), ==, 1);
	value_pair_free(list);
}

static void test_remove_existing(void)
{
	GList *list = make_list("a", "1");
	list = value_pair_add(list, "b", "2");
	list = value_pair_remove(list, "a");
	g_assert_cmpint(g_list_length(list), ==, 1);
	g_assert_null(value_pair_get_value(list, "a"));
	char *v = value_pair_get_value(list, "b");
	g_assert_cmpstr(v, ==, "2");
	g_free(v);
	value_pair_free(list);
}

static void test_remove_case_insensitive(void)
{
	GList *list = make_list("Key", "val");
	list = value_pair_remove(list, "KEY");
	g_assert_cmpint(g_list_length(list), ==, 0);
	value_pair_free(list);
}

/* ---------- value_pair_update ------------------------------------------- */

static void test_update_new_keys(void)
{
	GList *base = make_list("a", "1");
	GList *patch = make_list("b", "2");
	base = value_pair_update(base, patch);
	g_assert_cmpint(g_list_length(base), ==, 2);
	char *v = value_pair_get_value(base, "b");
	g_assert_cmpstr(v, ==, "2");
	g_free(v);
	value_pair_free(base);
	value_pair_free(patch);
}

static void test_update_overlapping_keys(void)
{
	GList *base = make_list("a", "old");
	GList *patch = make_list("a", "new");
	base = value_pair_update(base, patch);
	g_assert_cmpint(g_list_length(base), ==, 1);
	char *v = value_pair_get_value(base, "a");
	g_assert_cmpstr(v, ==, "new");
	g_free(v);
	value_pair_free(base);
	value_pair_free(patch);
}

static void test_update_mixed(void)
{
	GList *base = make_list("x", "1");
	base = value_pair_add(base, "y", "2");
	GList *patch = make_list("y", "99");
	patch = value_pair_add(patch, "z", "3");
	base = value_pair_update(base, patch);
	/* x stays, y updated, z added */
	g_assert_cmpint(g_list_length(base), ==, 3);
	char *vx = value_pair_get_value(base, "x");
	char *vy = value_pair_get_value(base, "y");
	char *vz = value_pair_get_value(base, "z");
	g_assert_cmpstr(vx, ==, "1");
	g_assert_cmpstr(vy, ==, "99");
	g_assert_cmpstr(vz, ==, "3");
	g_free(vx); g_free(vy); g_free(vz);
	value_pair_free(base);
	value_pair_free(patch);
}

/* ---------- value_pair_free --------------------------------------------- */

static void test_free_null(void)
{
	/* Must not crash on NULL */
	value_pair_free(NULL);
}

static void test_free_nonempty(void)
{
	GList *list = make_list("k", "v");
	list = value_pair_add(list, "k2", "v2");
	value_pair_free(list);
	/* No crash = pass */
}

/* ---------- main --------------------------------------------------------- */

int main(int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/value_pair/get/null_list",            test_get_null_list);
	g_test_add_func("/value_pair/get/missing_key",          test_get_missing_key);
	g_test_add_func("/value_pair/get/found",                test_get_found);
	g_test_add_func("/value_pair/get/case_insensitive",     test_get_case_insensitive);
	g_test_add_func("/value_pair/get/returns_copy",         test_get_returns_copy);

	g_test_add_func("/value_pair/add/to_null",              test_add_to_null);
	g_test_add_func("/value_pair/add/new_key",              test_add_new_key);
	g_test_add_func("/value_pair/add/replaces_existing",    test_add_replaces_existing);
	g_test_add_func("/value_pair/add/replaces_case_insens", test_add_replaces_case_insensitive);

	g_test_add_func("/value_pair/remove/null_list",         test_remove_null_list);
	g_test_add_func("/value_pair/remove/missing_key",       test_remove_missing_key);
	g_test_add_func("/value_pair/remove/existing",          test_remove_existing);
	g_test_add_func("/value_pair/remove/case_insensitive",  test_remove_case_insensitive);

	g_test_add_func("/value_pair/update/new_keys",          test_update_new_keys);
	g_test_add_func("/value_pair/update/overlapping",       test_update_overlapping_keys);
	g_test_add_func("/value_pair/update/mixed",             test_update_mixed);

	g_test_add_func("/value_pair/free/null",                test_free_null);
	g_test_add_func("/value_pair/free/nonempty",            test_free_nonempty);

	return g_test_run();
}
