src_inc     = include_directories('../src')
libxode_inc = include_directories('../modules/jabber/libxode/include')

test_mem_util = executable('test_mem_util',
  ['test_mem_util.c', '../src/mem_util.c'],
  include_directories: [src_inc],
  dependencies: [glib_dep, config_h_dep],
)
test('mem_util', test_mem_util)

test_value_pair = executable('test_value_pair',
  ['test_value_pair.c', '../src/value_pair.c', 'stubs.c'],
  include_directories: [src_inc],
  dependencies: [glib_dep, config_h_dep],
)
test('value_pair', test_value_pair)

# libxode_lib is defined in modules/jabber/libxode/meson.build.
# Meson subdir() scopes are inherited by the caller, so libxode_lib
# is visible here after subdir('modules') ran in the root meson.build.
test_jabber_str = executable('test_jabber_str',
  ['test_jabber_str.c'],
  include_directories: [libxode_inc],
  link_with: [libxode_lib],
  dependencies: [glib_dep],
)
test('jabber_str', test_jabber_str)
