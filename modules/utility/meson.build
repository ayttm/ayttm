src_inc = include_directories(join_paths(meson.project_source_root(), 'src'))

foreach mod_info : [
  ['notes',    'notes.c'],
  ['middle',   'middle.c'],
  ['rainbow',  'rainbow.c'],
  ['autotrans','autotrans.c'],
  ['custmsg',  'custom_message.c'],
]
  shared_module(mod_info[0],
    mod_info[1],
    include_directories: [top_inc, src_inc],
    dependencies: [gtk_dep, glib_dep],
    name_prefix: '',
    install: true,
    install_dir: pkglibdir,
  )
endforeach
