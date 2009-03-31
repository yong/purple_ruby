== INSTALLATION

Linux (Ubuntu):
---------------
apt-get install libpurple0 libpurple-dev
gem install yong-purplegw_ruby

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

sudo gem install yong-purplegw_ruby



