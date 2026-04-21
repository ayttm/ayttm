shared_module('aycryption',
  ['aycryption.c', 'select-keys.c'],
  include_directories: [top_inc, include_directories(join_paths(meson.project_source_root(), 'src'))],
  dependencies: [gtk_dep, glib_dep, gpgme_dep],
  c_args: ['-D_FILE_OFFSET_BITS=64'],
  name_prefix: '',
  install: true,
  install_dir: pkglibdir,
)
