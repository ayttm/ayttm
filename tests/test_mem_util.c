/*
 * tests/test_mem_util.c — unit tests for src/mem_util.c
 *
 * Tests: ay_string_append, ay_str_to_utf8, ay_utf8_to_str
 */

#include <glib.h>
#include "mem_util.h"

/* ---------- ay_string_append -------------------------------------------- */

static void test_string_append_basic(void)
{
	char *s = g_strdup("hello");
	char *r = ay_string_append(s, " world");
	/* s is consumed by ay_string_append; only use r */
	g_assert_cmpstr(r, ==, "hello world");
	g_free(r);
}

static void test_string_append_empty_suffix(void)
{
	char *s = g_strdup("hello");
	char *r = ay_string_append(s, "");
	g_assert_cmpstr(r, ==, "hello");
	g_free(r);
}

static void test_string_append_empty_base(void)
{
	char *s = g_strdup("");
	char *r = ay_string_append(s, "world");
	g_assert_cmpstr(r, ==, "world");
	g_free(r);
}

static void test_string_append_both_empty(void)
{
	char *s = g_strdup("");
	char *r = ay_string_append(s, "");
	g_assert_cmpstr(r, ==, "");
	g_free(r);
}

static void test_string_append_multiple(void)
{
	char *s = g_strdup("a");
	s = ay_string_append(s, "b");
	s = ay_string_append(s, "c");
	g_assert_cmpstr(s, ==, "abc");
	g_free(s);
}

/* ---------- ay_str_to_utf8 ----------------------------------------------- */

static void test_str_to_utf8_ascii(void)
{
	char *r = ay_str_to_utf8("hello");
	g_assert_cmpstr(r, ==, "hello");
	g_free(r);
}

static void test_str_to_utf8_high_byte(void)
{
	/* Latin-1 0xE9 (é) encodes as UTF-8 0xC3 0xA9 */
	const char in[] = { '\xE9', '\0' };
	char *r = ay_str_to_utf8(in);
	g_assert_nonnull(r);
	g_assert_cmphex((unsigned char)r[0], ==, 0xC3);
	g_assert_cmphex((unsigned char)r[1], ==, 0xA9);
	g_assert_cmpuint((unsigned char)r[2], ==, 0x00);
	g_free(r);
}

static void test_str_to_utf8_mixed(void)
{
	/* "caf" followed by Latin-1 0xE9 */
	const char in[] = { 'c', 'a', 'f', '\xE9', '\0' };
	char *r = ay_str_to_utf8(in);
	g_assert_nonnull(r);
	g_assert_cmphex((unsigned char)r[0], ==, 'c');
	g_assert_cmphex((unsigned char)r[1], ==, 'a');
	g_assert_cmphex((unsigned char)r[2], ==, 'f');
	g_assert_cmphex((unsigned char)r[3], ==, 0xC3);
	g_assert_cmphex((unsigned char)r[4], ==, 0xA9);
	g_assert_cmpuint((unsigned char)r[5], ==, 0x00);
	g_free(r);
}

static void test_str_to_utf8_empty(void)
{
	/* NOTE: implementation returns a non-heap "" literal for empty/NULL.
	 * Do NOT free the result in these cases. */
	char *r = ay_str_to_utf8("");
	g_assert_nonnull(r);
	g_assert_cmpuint(r[0], ==, '\0');
}

static void test_str_to_utf8_null(void)
{
	char *r = ay_str_to_utf8(NULL);
	g_assert_nonnull(r);
	g_assert_cmpuint(r[0], ==, '\0');
}

/* ---------- ay_utf8_to_str ----------------------------------------------- */

static void test_utf8_to_str_ascii(void)
{
	char *r = ay_utf8_to_str("hello");
	g_assert_cmpstr(r, ==, "hello");
	g_free(r);
}

static void test_utf8_to_str_two_byte(void)
{
	/* UTF-8 0xC3 0xA9 (é) decodes to Latin-1 0xE9 */
	const char in[] = { '\xC3', '\xA9', '\0' };
	char *r = ay_utf8_to_str(in);
	g_assert_nonnull(r);
	g_assert_cmphex((unsigned char)r[0], ==, 0xE9);
	g_assert_cmpuint((unsigned char)r[1], ==, 0x00);
	g_free(r);
}

static void test_utf8_to_str_empty(void)
{
	/* Same non-heap "" convention as ay_str_to_utf8; do not free. */
	char *r = ay_utf8_to_str("");
	g_assert_nonnull(r);
	g_assert_cmpuint(r[0], ==, '\0');
}

static void test_utf8_to_str_null(void)
{
	char *r = ay_utf8_to_str(NULL);
	g_assert_nonnull(r);
	g_assert_cmpuint(r[0], ==, '\0');
}

/* ---------- round-trip --------------------------------------------------- */

static void test_utf8_roundtrip_ascii(void)
{
	const char *orig = "hello world";
	char *utf8 = ay_str_to_utf8(orig);
	char *back = ay_utf8_to_str(utf8);
	g_assert_cmpstr(back, ==, orig);
	g_free(utf8);
	g_free(back);
}

static void test_utf8_roundtrip_latin1(void)
{
	/* "café" in Latin-1 */
	const char orig[] = { 'c', 'a', 'f', '\xE9', '\0' };
	char *utf8 = ay_str_to_utf8(orig);
	char *back = ay_utf8_to_str(utf8);
	g_assert_cmpstr(back, ==, orig);
	g_free(utf8);
	g_free(back);
}

static void test_utf8_roundtrip_high_range(void)
{
	/* Several high Latin-1 bytes */
	const char orig[] = { '\xC0', '\xE0', '\xFF', '\0' };
	char *utf8 = ay_str_to_utf8(orig);
	char *back = ay_utf8_to_str(utf8);
	g_assert_cmpstr(back, ==, orig);
	g_free(utf8);
	g_free(back);
}

/* ---------- main --------------------------------------------------------- */

int main(int argc, char **argv)
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/mem_util/string_append/basic",         test_string_append_basic);
	g_test_add_func("/mem_util/string_append/empty_suffix",  test_string_append_empty_suffix);
	g_test_add_func("/mem_util/string_append/empty_base",    test_string_append_empty_base);
	g_test_add_func("/mem_util/string_append/both_empty",    test_string_append_both_empty);
	g_test_add_func("/mem_util/string_append/multiple",      test_string_append_multiple);

	g_test_add_func("/mem_util/str_to_utf8/ascii",           test_str_to_utf8_ascii);
	g_test_add_func("/mem_util/str_to_utf8/high_byte",       test_str_to_utf8_high_byte);
	g_test_add_func("/mem_util/str_to_utf8/mixed",           test_str_to_utf8_mixed);
	g_test_add_func("/mem_util/str_to_utf8/empty",           test_str_to_utf8_empty);
	g_test_add_func("/mem_util/str_to_utf8/null",            test_str_to_utf8_null);

	g_test_add_func("/mem_util/utf8_to_str/ascii",           test_utf8_to_str_ascii);
	g_test_add_func("/mem_util/utf8_to_str/two_byte",        test_utf8_to_str_two_byte);
	g_test_add_func("/mem_util/utf8_to_str/empty",           test_utf8_to_str_empty);
	g_test_add_func("/mem_util/utf8_to_str/null",            test_utf8_to_str_null);

	g_test_add_func("/mem_util/roundtrip/ascii",             test_utf8_roundtrip_ascii);
	g_test_add_func("/mem_util/roundtrip/latin1",            test_utf8_roundtrip_latin1);
	g_test_add_func("/mem_util/roundtrip/high_range",        test_utf8_roundtrip_high_range);

	return g_test_run();
}
