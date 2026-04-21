/*
 * tests/stubs.c — minimal stubs for symbols called by tested source files
 * that live in modules not compiled into the test executables.
 */

#include <glib.h>

/*
 * escape_string is declared in src/util.h and called from
 * value_pair_print_values (which we do not test here).
 * Provide a no-op pass-through so the linker is satisfied.
 */
char *escape_string(const char *input)
{
	return g_strdup(input ? input : "");
}
