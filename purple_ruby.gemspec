
Gem::Specification.new do |s|
  s.name = %q{purple_ruby}
  s.version = "0.5.1"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["yong"]
  s.date = %q{2009-04-01}
  s.description = %q{A ruby gem to write server that sends and recives IM messages}
  s.email = %q{yong@intridea.com}
  s.extensions = ["ext/extconf.rb"]
  s.extra_rdoc_files = ["Manifest.txt", "History.txt", "README.txt"]
  s.files = ["ext/extconf.rb", "ext/purple_ruby.c", "ext/reconnect.c", "ext/account.c", "examples/purplegw_example.rb", "Manifest.txt", "History.txt", "README.txt", "Rakefile"]
  #s.has_rdoc = true
  s.homepage = %q{http://www.intridea.com}
  s.rdoc_options = ["--main", "README.txt"]
  s.require_paths = ["ext"]
  s.rubyforge_project = %q{purplegw_ruby}
  s.rubygems_version = %q{1.3.1}
  s.summary = %q{A ruby gem to write server that sends and recives IM messages}

  if s.respond_to? :specification_version then
    current_version = Gem::Specification::CURRENT_SPECIFICATION_VERSION
    s.specification_version = 2

    if Gem::Version.new(Gem::RubyGemsVersion) >= Gem::Version.new('1.2.0') then
      s.add_development_dependency(%q<hoe>, [">= 1.8.3"])
    else
      s.add_dependency(%q<hoe>, [">= 1.8.3"])
    end
  else
    s.add_dependency(%q<hoe>, [">= 1.8.3"])
  end
end
