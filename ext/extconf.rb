require 'mkmf'
$CFLAGS = "#{ENV['CFLAGS']} -Wall -O3 -g"
pkg_config 'purple'
pkg_config 'glib-2.0'
pkg_config 'gthread-2.0'
create_makefile('purple_ruby')
