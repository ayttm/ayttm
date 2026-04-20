shared_module('smiley-themer',
  'smiley-themer.c',
  include_directories: [top_inc, include_directories(join_paths(meson.project_source_root(), 'src'))],
  c_args: ['-DAYTTM_SMILEY_DIR="' + join_paths(get_option('datadir'), 'ayttm/smileys') + '"'],
  dependencies: [gtk_dep, glib_dep, xpm_lib],
  name_prefix: '',
  install: true,
  install_dir: pkglibdir,
)
