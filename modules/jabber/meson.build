subdir('libjabber')
subdir('libxode')

shared_module('jabber',
  ['jabber.c', 'libEBjabber.c'],
  include_directories: [
    top_inc,
    include_directories(join_paths(meson.project_source_root(), 'src')),
    include_directories('libjabber/include'),
    include_directories('libxode/include'),
  ],
  c_args: ['-DJDEBUG'],
  dependencies: [gtk_dep, glib_dep, openssl_dep],
  link_with: [libjabber_lib, libxode_lib],
  name_prefix: '',
  install: true,
  install_dir: pkglibdir,
)
