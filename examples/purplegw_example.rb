#
#Example Usage:
#
#Start the daemon and receive IM:
#$ruby examples/purplegw_example.rb prpl-msn user@hotmail.com password prpl-jabber user@gmail.com password
#
#Send im:
#$ irb
#irb(main):001:0> require 'lib/purplegw_ruby'
#irb(main):007:0> PurpleGW.deliver 'prpl-jabber', 'friend@gmail.com', 'hello worlds!'
#

require 'hpricot'
require 'socket'
require File.expand_path(File.join(File.dirname(__FILE__), '../ext/purple_ruby'))

class PurpleGWExample
  SERVER_IP = "127.0.0.1"
  SERVER_PORT = 9876

  def start configs
    PurpleRuby.init false #use 'true' if you want to see the debug messages
    
    puts "Available protocols:", PurpleRuby.list_protocols
    
    accounts = {}
    configs.each {|config|
      account = PurpleRuby.login(config[:protocol], config[:username], config[:password])
      accounts[config[:protocol]] = account
    }
    
    #handle incoming im messages
    PurpleRuby.watch_incoming_im do |receiver, sender, message|
      sender = sender[0...sender.index('/')] if sender.index('/') #discard anything after '/'
      text = (Hpricot(message)).to_plain_text
      puts "recv: #{receiver}, #{sender}, #{text}"
    end
    
    PurpleRuby.watch_signed_on_event do |acc| 
      puts "signed on: #{acc.username}"
    end
    
    PurpleRuby.watch_connection_error do |acc| 
      raise "connection_error: #{acc.username}"
    end
    
    PurpleRuby.watch_notify_message do |type, title, primary, secondary|
      puts "notify: #{type} #{title} #{primary} #{secondary}"
    end
    
    PurpleRuby.watch_request do |title, primary, secondary, who|
      puts "request: #{title} #{primary} #{secondary} #{who}"
      true #return 'true' to accept an request, return 'false' to ignore it
    end
    
    #listen a tcp port, parse incoming data and send it out.
    #We assume the incoming data is in the following format:
    #<protocol> <user> <message>
    PurpleRuby.watch_incoming_ipc(SERVER_IP, SERVER_PORT) do |data|
      protocol, user, message = data.split(",").collect{|x| x.chomp.strip!}
      puts "send: #{protocol}, #{user}, #{message}"
      accounts[protocol].send_im(user, message)
    end
        
    PurpleRuby.main_loop_run
  end
  
  def self.deliver(protocol, to_users, message)
    to_users = [to_users] unless to_users.is_a?(Array)      
    to_users.each do |user|
      t = TCPSocket.new(SERVER_IP, SERVER_PORT)
      t.print "#{protocol} #{user} #{message}\n"
      t.close
    end
  end
end

if ARGV.length >= 3
  configs = []
  configs << {:protocol => ARGV[0], :username => ARGV[1], :password => ARGV[2]}
  configs << {:protocol => ARGV[3], :username => ARGV[4], :password => ARGV[5]} if ARGV.length >= 6
  #add more accounts here if you like
  PurpleGWExample.new.start configs
end

