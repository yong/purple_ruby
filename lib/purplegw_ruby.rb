require File.expand_path(File.join(File.dirname(__FILE__), 'purplegw_ext'))

class PurpleGW
  def start protocol, username, password, friend
    PurpleGW.init false
    
    account = PurpleGW.login(protocol, username, password)
    
    #handle incoming im messages
    PurpleGW.watch_incoming_im do |receiver, sender, message| 
      puts "recv: #{receiver}, #{sender}, #{message}"
    end
    
    PurpleGW.watch_signed_on_event do |acc| 
      puts "signed on: #{acc.username}"
    end
    
    #listen a tcp port, parse incoming data and send it out
    PurpleGW.watch_incoming_ipc("127.0.0.1", 9877) do |data|
      puts "send: #{data}"
      account.send_im(friend, data)
    end
    
    PurpleGW.main_loop_run
  end
end

#prpl-jabber
PurpleGW.new.start ARGV[0], ARGV[1], ARGV[2], ARGV[3]
