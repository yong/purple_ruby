
EXT = "ext/ruburple_ext.#{Config::CONFIG['DLEXT']}"

task :default => EXT

file EXT => ["ext/extconf.rb", "ext/purple_ruby.c"] do
  Dir.chdir "ext" do
    ruby "extconf.rb"
    sh "make"
  end
end
