== OVERVIEW

purple_ruby is a ruby gem to write servers that send and recive IM messages. It uses libpurple ( http://developer.pidgin.im/wiki/WhatIsLibpurple ) and therforce supports all protocols that Pidgin/Adium supports (MSN/Gtalk/Yahoo/AIM/ICQ etc).

For MSN, we recommend msn-pecan ( http://code.google.com/p/msn-pecan/ ), which is more more stable than official MSN plugin.

Please check examples/purplegw_example.rb for details. Bascially you just tell it what to do when an IM was received, and there is an embedded tcp 'proxy' which allows you send IM messages.

Why not "ruburple"? I have used ruburple ( http://rubyforge.org/projects/ruburple ), but found it blocks a lot. libpurple needs to run its own event loop which interferes with ruby's green thread model. Ruburple's author has done lots of hard work to workaround the problem ( http://rubyforge.org/pipermail/ruburple-development/2007-June/000005.html ), but it does not work well.

== INSTALLATION

Ubuntu:
---------------
sudo apt-get install libpurple0 libpurple-dev
gem sources -a http://gems.github.com (you only have to do this once)
sudo gem install yong-purple_ruby

Redhat/Centos
---------------
wget -O /etc/yum.repos.d/pidgin.repo http://rpm.pidgin.im/centos/pidgin.repo
yum -y install glib2-devel libpurple-devel
gem sources -a http://gems.github.com (you only have to do this once)
sudo gem install yong-purple_ruby

OSX:
----
sudo port -d selfupdate
sudo port install gnutls
		(wait forever....)
sudo port install nss
		(wait forever....)
wget http://downloads.sourceforge.net/pidgin/pidgin-2.5.7.tar.bz2
tar xvjf pidgin-2.5.7.tar.bz2
cd pidgin-2.5.7
./configure --disable-gtkui --disable-screensaver --disable-consoleui --disable-sm --disable-perl --disable-tk --disable-tcl --disable-gstreamer --disable-schemas-install --disable-gestures --disable-cap --disable-gevolution --disable-gtkspell --disable-startup-notification --disable-avahi --disable-nm --disable-dbus --disable-meanwhile
make
		(wait forever...)
sudo make install

edit your ~/.bash_profile and add this line
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig

gem sources -a http://gems.github.com (you only have to do this once)
sudo gem install yong-purple_ruby

== Copyright

purple_ruby is Copyright (c) 2009 Xue Yong Zhi and Intridea, Inc. ( http://intridea.com ), released under the GPL License.




