require 'rubygems'
spec = Gem::Specification.new do |s|
  s.name = "unnature"
  s.version = "0.0.1"
  s.author = "David M Michael"
  s.email = "david@unnature.net"
  s.homepage = "http://unnature.net"
  s.platform = Gem::Platform::RUBY
  s.summary = "Unnature extentions to Marionette."
  candidates = Dir.glob("{ext,../lib,../marionette,../stk,../test,lib}/**/*")
  s.files = candidates.delete_if do |item|
              item.include?(".svn")
              item.include?("marionette.rb")
            end
  s.require_path = "lib"
  s.extensions = ["ext/extconf.rb"]
end
if $0 == __FILE__
  Gem::manage_gems
  Gem::Builder.new(spec).build
end