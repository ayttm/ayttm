libayttmgtk_sources = files(
  'html_text_buffer.c',
  'about_window.c',
  'gtkspell.c',
  'gtkutils.c',
  'gtk_tree_view_tooltip.c',
  'log_window.C',
  'prefs_window.c',
)

libayttmgtk = static_library('ayttmgtk',
  libayttmgtk_sources,
  include_directories: [include_directories('..'), top_inc],
  dependencies: [gtk_dep, glib_dep, enchant_dep],
)

libayttmgtk_dep = declare_dependency(
  link_with: libayttmgtk,
  dependencies: [gtk_dep],
)
