require File.expand_path(File.join(File.dirname(__FILE__), 'purplegw_ext'))

class PurpleGW
  def start protocol, username, password
    PurpleGW.init false
    
    PurpleGW.watch_incoming_messages do |receiver, sender, message| 
      puts "#{receiver}, #{sender}, #{message}"
    end
    
    account = PurpleGW.login(protocol, username, password)
    
    PurpleGW.watch_incoming_connections(9877) do |data|
      account.send_im('xue.yong.zhi@gmail.com', data)
    end
    
    PurpleGW.main_loop_run
  end
end

#prpl-jabber
PurpleGW.new.start ARGV[0], ARGV[1], ARGV[2]
