subdir('gtk')

flex  = find_program('flex',  required: true)
bison = find_program('bison', required: true)

# Generate defaults.h
defaults_conf = configuration_data()
defaults_conf.set_quoted('MODULE_DIR', join_paths(get_option('libdir'), 'ayttm'))
defaults_conf.set_quoted('AYTTMRC',   join_paths(get_option('sysconfdir'), 'ayttmrc'))
defaults_h = configure_file(
  output: 'defaults.h',
  configuration: defaults_conf,
  format: 'c',
)

# Bison/flex generated sources
account_parser = custom_target('account_parser',
  input:   'account_parser.y',
  output:  ['account_parser.c', 'account_parser.h'],
  command: [bison, '-d', '-p', 'account', '@INPUT@', '-o', '@OUTPUT0@', '--defines=@OUTPUT1@'],
)

contact_parser = custom_target('contact_parser',
  input:   'contact_parser.y',
  output:  ['contact_parser.c', 'contact_parser.h'],
  command: [bison, '-d', '-p', 'contact', '@INPUT@', '-o', '@OUTPUT0@', '--defines=@OUTPUT1@'],
)

account_scanner = custom_target('account_scanner',
  input:   'account_scanner.l',
  output:  'account_scanner.c',
  command: [flex, '-i', '-t', '-Paccount', '@INPUT@'],
  capture: true,
)

contact_scanner = custom_target('contact_scanner',
  input:   'contact_scanner.l',
  output:  'contact_scanner.c',
  command: [flex, '-i', '-t', '-Pcontact', '@INPUT@'],
  capture: true,
)

ayttm_sources = files(
  'main.c',
  'contactlist.c',
  'edit_local_accounts.c',
  'service.c',
  'status.c',
  'account.c',
  'info_window.c',
  'chat_window.c',
  'util.c',
  'add_contact_window.c',
  'edit_contact_window.c',
  'edit_account_window.c',
  'sound.c',
  'dialog.c',
  'prefs.c',
  'away_window.c',
  'browser.c',
  'message_parse.c',
  'activity_bar.C',
  'spellcheck.C',
  'video.c',
  'usercount.c',
  'conversation.c',
  'value_pair.c',
  'edit_group_window.c',
  'logs.c',
  'trigger.c',
  'console_session.c',
  'input_list.c',
  'nomodule.c',
  'plugin_api.c',
  'plugin.c',
  'file_select.c',
  'contact_actions.c',
  'smileys.c',
  'help_menu.c',
  'crash.c',
  'mem_util.c',
  'offline_queue_mgmt.c',
  'image_window.c',
  'action.c',
  'about.c',
  'messages.c',
  'edit_list_window.c',
  'flash_window_title.c',
  'icons.c',
  'charconv.c',
  'ayttm_tray.c',
  'auto_complete.c',
)

ayttm_c_args = [
  '-DLOCALEDIR="' + join_paths(get_option('prefix'), get_option('localedir')) + '"',
  '-D_REENTRANT',
  '-DIMG_DIR="' + join_paths(get_option('datadir'), 'pixmaps') + '/"',
]

executable('ayttm',
  [ayttm_sources,
   account_parser, contact_parser,
   account_scanner, contact_scanner,
   defaults_h, config_h],
  include_directories: [top_inc, libproxy_inc],
  dependencies: [
    glib_dep,
    gtk_dep,
    openssl_dep,
    enchant_dep,
    thread_dep,
    x11_dep,
    xss_dep,
    ltdl_dep,
    audiofile_dep,
    libproxy_dep,
    config_h_dep,
  ],
  link_with: [libayttmgtk],
  c_args: ayttm_c_args,
  cpp_args: ayttm_c_args,
  export_dynamic: true,
  install: true,
)
