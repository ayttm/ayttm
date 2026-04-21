/*
 * tests/test_jabber_str.c — unit tests for modules/jabber/libxode/str.c
 *
 * Tests: j_strdup, j_strcmp, j_strcasecmp, j_strncmp, j_strncasecmp,
 *        j_strlen, j_atoi
 *
 * Note on j_strcmp semantics: returns 0 for equal, -1 for any difference
 * (it is NOT a standard three-way comparison — do not test for >0 or <0).
 */

#include <glib.h>
#include "lib.h"

/* ---------- j_strdup ---------------------------------------------------- */

static void test_strdup_null(void)
{
	g_assert_null(j_strdup(NULL));
}

static void test_strdup_empty(void)
{
	char *r = j_strdup("");
	g_assert_nonnull(r);
	g_assert_cmpstr(r, ==, "");
	g_free(r);
}

static void test_strdup_normal(void)
{
	char *r = j_strdup("hello");
	g_assert_cmpstr(r, ==, "hello");
	g_free(r);
}

static void test_strdup_returns_copy(void)
{
	const char *orig = "test";
	char *copy = j_strdup(orig);
	g_assert_true(copy != orig);	/* must be a new allocation */
	g_free(copy);
}

/* ---------- j_strcmp ---------------------------------------------------- */

static void test_strcmp_equal(void)
{
	g_assert_cmpint(j_strcmp("abc", "abc"), ==, 0);
}

static void test_strcmp_different(void)
{
	g_assert_cmpint(j_strcmp("abc", "xyz"), ==, -1);
}

static void test_strcmp_prefix(void)
{
	/* "ab" != "abc" */
	g_assert_cmpint(j_strcmp("ab", "abc"), ==, -1);
}

static void test_strcmp_null_first(void)
{
	g_assert_cmpint(j_strcmp(NULL, "abc"), ==, -1);
}

static void test_strcmp_null_second(void)
{
	g_assert_cmpint(j_strcmp("abc", NULL), ==, -1);
}

static void test_strcmp_both_null(void)
{
	g_assert_cmpint(j_strcmp(NULL, NULL), ==, -1);
}

static void test_strcmp_empty_equal(void)
{
	g_assert_cmpint(j_strcmp("", ""), ==, 0);
}

/* j_strcmp is case-sensitive */
static void test_strcmp_case_sensitive(void)
{
	g_assert_cmpint(j_strcmp("abc", "ABC"), ==, -1);
}

/* ---------- j_strcasecmp ------------------------------------------------ */

static void test_strcasecmp_equal_same_case(void)
{
	g_assert_cmpint(j_strcasecmp("hello", "hello"), ==, 0);
}

static void test_strcasecmp_equal_mixed_case(void)
{
	g_assert_cmpint(j_strcasecmp("Hello", "hELLO"), ==, 0);
}

static void test_strcasecmp_different(void)
{
	g_assert_cmpint(j_strcasecmp("abc", "xyz"), !=, 0);
}

static void test_strcasecmp_null_first(void)
{
	g_assert_cmpint(j_strcasecmp(NULL, "abc"), ==, -1);
}

static void test_strcasecmp_null_second(void)
{
	g_assert_cmpint(j_strcasecmp("abc", NULL), ==, -1);
}

/* ---------- j_strncmp --------------------------------------------------- */

static void test_strncmp_equal_prefix(void)
{
	g_assert_cmpint(j_strncmp("abcdef", "abcXXX", 3), ==, 0);
}

static void test_strncmp_different_prefix(void)
{
	g_assert_cmpint(j_strncmp("abcdef", "abddef", 3), !=, 0);
}

static void test_strncmp_zero_len(void)
{
	g_assert_cmpint(j_strncmp("abc", "xyz", 0), ==, 0);
}

static void test_strncmp_null_first(void)
{
	g_assert_cmpint(j_strncmp(NULL, "abc", 3), ==, -1);
}

static void test_strncmp_null_second(void)
{
	g_assert_cmpint(j_strncmp("abc", NULL, 3), ==, -1);
}

/* ---------- j_strncasecmp ----------------------------------------------- */

static void test_strncasecmp_equal_prefix_mixed_case(void)
{
	g_assert_cmpint(j_strncasecmp("AbCdef", "abcXXX", 3), ==, 0);
}

static void test_strncasecmp_different_prefix(void)
{
	g_assert_cmpint(j_strncasecmp("AbCdef", "abddef", 3), !=, 0);
}

static void test_strncasecmp_null_first(void)
{
	g_assert_cmpint(j_strncasecmp(NULL, "abc", 3), ==, -1);
}

static void test_strncasecmp_null_second(void)
{
	g_assert_cmpint(j_strncasecmp("abc", NULL, 3), ==, -1);
}

/* ---------- j_strlen ---------------------------------------------------- */

static void test_strlen_null(void)
{
	g_assert_cmpint(j_strlen(NULL), ==, 0);
}

static void test_strlen_empty(void)
{
	g_assert_cmpint(j_strlen(""), ==, 0);
}

static void test_strlen_normal(void)
{
	g_assert_cmpint(j_strlen("hello"), ==, 5);
}

/* ---------- j_atoi ------------------------------------------------------ */

static void test_atoi_null_returns_default(void)
{
	g_assert_cmpint(j_atoi(NULL, 42), ==, 42);
}

static void test_atoi_valid_number(void)
{
	g_assert_cmpint(j_atoi("123", 0), ==, 123);
}

static void test_atoi_zero(void)
{
	g_assert_cmpint(j_atoi("0", 99), ==, 0);
}

static void test_atoi_negative(void)
{
	g_assert_cmpint(j_atoi("-5", 0), ==, -5);
}

/*
 * j_atoi delegates to atoi() for non-NULL input: atoi("abc") == 0,
 * NOT the default — the default is only used when input is NULL.
 */
static void test_atoi_invalid_string_returns_zero_not_default(void)
{
	g_assert_cmpint(j_atoi("abc", 99), ==, 0);
}

static void test_atoi_leading_whitespace(void)
{
	/* atoi skips leading whitespace */
	g_assert_cmpint(j_atoi("  7", 0), ==, 7);
}

/* ---------- main --------------------------------------------------------- */

int main(int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/jabber_str/strdup/null",               test_strdup_null);
	g_test_add_func("/jabber_str/strdup/empty",              test_strdup_empty);
	g_test_add_func("/jabber_str/strdup/normal",             test_strdup_normal);
	g_test_add_func("/jabber_str/strdup/returns_copy",       test_strdup_returns_copy);

	g_test_add_func("/jabber_str/strcmp/equal",              test_strcmp_equal);
	g_test_add_func("/jabber_str/strcmp/different",          test_strcmp_different);
	g_test_add_func("/jabber_str/strcmp/prefix",             test_strcmp_prefix);
	g_test_add_func("/jabber_str/strcmp/null_first",         test_strcmp_null_first);
	g_test_add_func("/jabber_str/strcmp/null_second",        test_strcmp_null_second);
	g_test_add_func("/jabber_str/strcmp/both_null",          test_strcmp_both_null);
	g_test_add_func("/jabber_str/strcmp/empty_equal",        test_strcmp_empty_equal);
	g_test_add_func("/jabber_str/strcmp/case_sensitive",     test_strcmp_case_sensitive);

	g_test_add_func("/jabber_str/strcasecmp/equal_same",     test_strcasecmp_equal_same_case);
	g_test_add_func("/jabber_str/strcasecmp/equal_mixed",    test_strcasecmp_equal_mixed_case);
	g_test_add_func("/jabber_str/strcasecmp/different",      test_strcasecmp_different);
	g_test_add_func("/jabber_str/strcasecmp/null_first",     test_strcasecmp_null_first);
	g_test_add_func("/jabber_str/strcasecmp/null_second",    test_strcasecmp_null_second);

	g_test_add_func("/jabber_str/strncmp/equal_prefix",      test_strncmp_equal_prefix);
	g_test_add_func("/jabber_str/strncmp/different_prefix",  test_strncmp_different_prefix);
	g_test_add_func("/jabber_str/strncmp/zero_len",          test_strncmp_zero_len);
	g_test_add_func("/jabber_str/strncmp/null_first",        test_strncmp_null_first);
	g_test_add_func("/jabber_str/strncmp/null_second",       test_strncmp_null_second);

	g_test_add_func("/jabber_str/strncasecmp/equal_mixed",   test_strncasecmp_equal_prefix_mixed_case);
	g_test_add_func("/jabber_str/strncasecmp/different",     test_strncasecmp_different_prefix);
	g_test_add_func("/jabber_str/strncasecmp/null_first",    test_strncasecmp_null_first);
	g_test_add_func("/jabber_str/strncasecmp/null_second",   test_strncasecmp_null_second);

	g_test_add_func("/jabber_str/strlen/null",               test_strlen_null);
	g_test_add_func("/jabber_str/strlen/empty",              test_strlen_empty);
	g_test_add_func("/jabber_str/strlen/normal",             test_strlen_normal);

	g_test_add_func("/jabber_str/atoi/null_default",         test_atoi_null_returns_default);
	g_test_add_func("/jabber_str/atoi/valid",                test_atoi_valid_number);
	g_test_add_func("/jabber_str/atoi/zero",                 test_atoi_zero);
	g_test_add_func("/jabber_str/atoi/negative",             test_atoi_negative);
	g_test_add_func("/jabber_str/atoi/invalid_is_zero",      test_atoi_invalid_string_returns_zero_not_default);
	g_test_add_func("/jabber_str/atoi/leading_whitespace",   test_atoi_leading_whitespace);

	return g_test_run();
}
