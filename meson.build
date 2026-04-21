project('ayttm', ['c', 'cpp'],
  version: '0.6.3',
  license: 'GPL-2.0-or-later',
  meson_version: '>= 0.56.0',
  default_options: [
    'c_std=c99',
    'cpp_std=c++11',
    'warning_level=1',
    'b_pie=true',
  ],
)

cc = meson.get_compiler('c')

# Core dependencies
glib_dep      = dependency('glib-2.0', version: '>= 2.32')
gtk_dep       = dependency('gtk+-2.0', version: '>= 2.14.0')
openssl_dep   = dependency('openssl', required: false)
enchant_dep   = dependency('enchant-2', required: false)
thread_dep    = dependency('threads')
x11_dep       = dependency('x11', required: false)
xss_dep       = dependency('xscrnsaver', required: false)

# Libraries found by linker search
ltdl_dep      = cc.find_library('ltdl')
xpm_lib       = cc.find_library('Xpm', required: false)

# Optional module dependencies
gpgme_dep     = dependency('gpgme', required: false)
audiofile_dep = cc.find_library('audiofile', required: false)

# Build config.h
conf = configuration_data()
conf.set('HAVE_GLIB', 1)
conf.set('HAVE_GTK', 1)
conf.set('_REENTRANT', 1)
conf.set_quoted('PACKAGE', 'ayttm')
conf.set_quoted('VERSION', meson.project_version())
conf.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
if openssl_dep.found()
  if not openssl_dep.version().version_compare('>=1.1.1')
    error('OpenSSL >= 1.1.1 is required (TLS 1.3 support needs 1.1.1+).')
  endif
  conf.set('HAVE_OPENSSL', 1)
endif
if enchant_dep.found()
  conf.set('HAVE_LIBENCHANT', 1)
endif
if xss_dep.found()
  conf.set('HAVE_XSS', 1)
endif
if xpm_lib.found()
  conf.set('HAVE_XPM', 1)
endif

config_h = configure_file(output: 'config.h', configuration: conf)
config_h_dep = declare_dependency(sources: config_h)

# Top-level include dirs exported to subdirs
top_inc     = include_directories('.')
libproxy_inc = include_directories('libproxy')

# Plugin install directory
pkglibdir = join_paths(get_option('libdir'), 'ayttm')
pkgdatadir = join_paths(get_option('datadir'), 'ayttm')

subdir('libproxy')
subdir('src')
subdir('modules')
subdir('pixmaps')
subdir('sounds')
subdir('tests')
