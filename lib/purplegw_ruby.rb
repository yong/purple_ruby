#Example Usage:
#ruby purplegw_ruby.rb prpl-jabber user@gmail.com password prpl-msn user@hotmail.com password

require 'hpricot'
require File.expand_path(File.join(File.dirname(__FILE__), 'purplegw_ext'))

class PurpleGW
  SERVER_IP = "127.0.0.1"
  SERVER_PORT = 9876

  def start configs
    PurpleGW.init false
    
    accounts = {}
    configs.each {|config|
      account = PurpleGW.login(config[:protocol], config[:username], config[:password])
      accounts[config[:protocol]] = account
    }
    
    #handle incoming im messages
    PurpleGW.watch_incoming_im do |receiver, sender, message|
      sender = sender[0...sender.index('/')] if sender.index('/') #discard anything after '/'
      text = (Hpricot(message)).to_plain_text
      puts "recv: #{receiver}, #{sender}, #{text}"
    end
    
    PurpleGW.watch_signed_on_event do |acc| 
      puts "signed on: #{acc.username}"
    end
    
    #listen a tcp port, parse incoming data and send it out.
    #We assume the incoming data is in the following format:
    #<protocol> <user> <message>
    PurpleGW.watch_incoming_ipc(SERVER_IP, SERVER_PORT) do |data|
      first_space = data.index(' ')
      second_space = data.index(' ', first_space + 1)
      protocol = data[0...first_space]
      user = data[(first_space+1)...second_space]
      message = data[(second_space+1)...-1]
      puts "send: #{protocol}, #{user}, #{message}"
      accounts[protocol].send_im(user, message)
    end
    
    trap("INT") {
      #TODO ctrl-c can not be deteced until a message is coming
      puts 'Ctrl-C, quit...'
      PurpleGW.main_loop_stop
    }
    
    PurpleGW.main_loop_run
  end
  
  def self.deliver(protocol, message, to_users, opts={})
    to_users = [to_users] unless to_users.is_a?(Array)      
    to_users.each do |user|
      t = TCPSocket.new(SERVER_IP, SERVER_PORT)
      t.print "#{protocol} #{user} #{message}\n"
      t.close
    end
  end
end

accounts = [{:protocol => ARGV[0], :username => ARGV[1], :password => ARGV[2]},
            {:protocol => ARGV[3], :username => ARGV[4], :password => ARGV[5]}]
PurpleGW.new.start accounts
