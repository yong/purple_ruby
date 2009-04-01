== OVERVIEW

purple_ruby is a ruby gem to write servers that send and recive IM messages. It uses libpurple (http://developer.pidgin.im/wiki/WhatIsLibpurple) and therforce supports all protocols that Pidgin/Adium supports (MSN/Gtalk/Yahoo/AIM/ICQ etc).

Please check examples/purplegw_example.rb for details. Bascially you just tell it what to do when an IM was received, and there is an embedded tcp 'proxy' which allows you send IM messages.

Why not "ruburple"? I have used ruburple (http://rubyforge.org/projects/ruburple), but found it blocks a lot. libpurple needs to run its own event loop which interferes with ruby's green thread model. Ruburple's author has done lots of hard work to workaround the problem (http://rubyforge.org/pipermail/ruburple-development/2007-June/000005.html), but it does not work well.

== INSTALLATION

Linux (Ubuntu):
---------------
apt-get install libpurple0 libpurple-dev
gem install yong-purple_ruby

OSX:
----
sudo port -d selfupdate
sudo port install gnutls
		(wait forever....)
sudo port install nss
		(wait forever....)
wget http://downloads.sourceforge.net/pidgin/pidgin-2.5.5.tar.bz2
tar xvjf pidgin-2.5.5.tar.bz2
cd pidgin-2.5.5
./configure --disable-gtkui --disable-screensaver --disable-consoleui --disable-sm --disable-perl --disable-tk --disable-tcl --disable-gstreamer --disable-schemas-install --disable-gestures --disable-cap --disable-gevolution --disable-gtkspell --disable-startup-notification --disable-avahi --disable-nm --disable-dbus --disable-meanwhile
make
		(wait forever...)
sudo make install

edit your ~/.bash_profile and add this line
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig

sudo gem install yong-purple_ruby




