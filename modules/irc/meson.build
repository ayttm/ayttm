subdir('libirc')

shared_module('irc',
  'irc.c',
  include_directories: [
    top_inc,
    include_directories('libirc'),
    include_directories(join_paths(meson.project_source_root(), 'src')),
  ],
  dependencies: [gtk_dep, glib_dep],
  link_with: [libirc_lib],
  name_prefix: '',
  install: true,
  install_dir: pkglibdir,
)
