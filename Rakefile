require 'rubygems'
require 'hoe'

EXT = "ext/ruburple_ext.#{Hoe::DLEXT}"

Hoe.new('purple_ruby', '0.2.1') do |p|
  p.author = 'yong'
  p.email = 'yong@intridea.com'
  p.url = 'http://www.intridea.com'
  p.summary = 'A ruby gem to write server that sends and recives IM messages'
  p.description = 'A ruby gem to write server that sends and recives IM messages'
  
  p.spec_extras[:extensions] = "ext/extconf.rb"
  p.clean_globs << EXT << "ext/*.o" << "ext/Makefile"
end

task :test => EXT

file EXT => ["ext/extconf.rb", "ext/purple_ruby.c"] do
  Dir.chdir "ext" do
    ruby "extconf.rb"
    sh "make"
  end
end
