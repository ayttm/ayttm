libproxy_sources = files(
  'proxy.c',
  'common.c',
  'networking.c',
  'ssl.c',
  'ssl_certificate.c',
)

libproxy_inc_dir = include_directories('.')

libproxy_lib = static_library('proxy',
  libproxy_sources,
  include_directories: [libproxy_inc_dir, top_inc],
  dependencies: [glib_dep, openssl_dep],
)

libproxy_dep = declare_dependency(
  link_with: libproxy_lib,
  include_directories: libproxy_inc_dir,
  dependencies: [glib_dep, openssl_dep],
)
