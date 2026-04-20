# Always-built modules
subdir('importers')
subdir('utility')
subdir('image_filter')

# Conditionally built based on available deps
subdir('irc')
subdir('jabber')
subdir('smtp')

if xpm_lib.found()
  subdir('smileys')
endif

if gpgme_dep.found()
  subdir('aycryption')
endif
