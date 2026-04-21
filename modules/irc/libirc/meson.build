libirc_lib = static_library('irc-impl',
  ['irc_message.c', 'libirc.c', 'ctcp.c'],
  include_directories: [top_inc, include_directories(join_paths(meson.project_source_root(), 'src'))],
  dependencies: [glib_dep],
)
