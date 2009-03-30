require 'rubygems'
require 'hoe'

EXT = "ext/ruburple_ext.#{Hoe::DLEXT}"

Hoe.new('purplegw_ruby', '0.1.0') do |p|
  p.author = 'yong'
  p.email = 'yong@intridea.com'
  p.url = 'http://www.intridea.com'
  p.summary = 'ruby IM gateway based on libpurple'
  p.description = 'ruby IM gateway based on libpurple'
  
  p.spec_extras[:extensions] = "ext/extconf.rb"
  p.clean_globs << EXT << "ext/*.o" << "ext/Makefile"
end

task :test => EXT

file EXT => ["ext/extconf.rb", "ext/purplegw_ext.c"] do
  Dir.chdir "ext" do
    ruby "extconf.rb"
    sh "make"
    sh "cp purplegw_ext.so ../lib"
  end
end
