shared_module('smtp',
  'smtp.c',
  include_directories: [top_inc, include_directories(join_paths(meson.project_source_root(), 'src'))],
  dependencies: [gtk_dep, glib_dep],
  name_prefix: '',
  install: true,
  install_dir: pkglibdir,
)
