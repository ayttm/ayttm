src_inc = include_directories(join_paths(meson.project_source_root(), 'src'))

foreach mod : ['import_gaim', 'import_gnomeicu', 'importlicq', 'import_everybuddy', 'importicq']
  shared_module(mod,
    mod + '.c',
    include_directories: [top_inc, src_inc],
    dependencies: [gtk_dep, glib_dep],
    name_prefix: '',
    install: true,
    install_dir: pkglibdir,
  )
endforeach
