libjabber_lib = static_library('jabber-impl',
  ['jid.c', 'log.c', 'pproxy.c', 'jutil.c', 'rate.c', 'jpacket.c', 'jconn.c'],
  include_directories: [
    include_directories('include'),
    include_directories('include/jabber'),
    include_directories('../libxode/include'),
    top_inc,
  ],
  dependencies: [glib_dep, openssl_dep],
)
