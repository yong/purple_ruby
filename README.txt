== OVERVIEW

purple_ruby is a ruby gem to write servers that send and recive IM messages. It uses libpurple ( http://developer.pidgin.im/wiki/WhatIsLibpurple ) and therforce supports all protocols that Pidgin/Adium supports (MSN/Gtalk/Yahoo/AIM/ICQ etc).

For MSN, we recommend msn-pecan ( http://code.google.com/p/msn-pecan/ ), which is more more stable than official MSN plugin.

Please check examples/purplegw_example.rb for details. Bascially you just tell it what to do when an event happens (e.g. IM was received), and there is an embedded tcp 'proxy' which allows you send IM messages.

Why not "ruburple"? I have used ruburple ( http://rubyforge.org/projects/ruburple ), but found it blocks a lot. libpurple needs to run its own event loop which interferes with ruby's green thread model. Ruburple's author has done lots of hard work to workaround the problem ( http://rubyforge.org/pipermail/ruburple-development/2007-June/000005.html ), but it does not work well.

== INSTALLATION

RubyGems v1.3.6 and up is required.

Ubuntu:
---------------
sudo apt-get install libpurple0 libpurple-dev
sudo gem install purple_ruby

Redhat/Centos
---------------
sudo wget -O /etc/yum.repos.d/pidgin.repo http://rpm.pidgin.im/centos/pidgin.repo
sudo yum -y install glib2-devel libpurple-devel
sudo gem install purple_ruby

OSX:
----
sudo port install pidgin
sudo gem install purple_ruby

== Notes

If you have problems login into gtalk with the error "NotImplementedError: method `respond_to?' called on terminated object (0x1018c9af0)", it's highly possible that the library is conflicted with libxml-ruby gem. Try to upgrade libxml2 to the latest version and recompile libxml-ruby will fix the problem

== Copyright

purple_ruby is Copyright (c) 2009-2010 Xue Yong Zhi and Intridea, Inc. ( http://intridea.com ), released under the GPL License.
