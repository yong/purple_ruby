require File.expand_path(File.join(File.dirname(__FILE__), 'purplegw_ext'))

class PurpleGW
  def start protocol, username, password
    PurpleGW.init false
    
    PurpleGW.subscribe('signed-on') do |c| 
      puts "signed on #{c}"
    end
    
    PurpleGW.login protocol, username, password
    PurpleGW.main_loop_run
    
    #Ruburple::subscribe(:received_im_msg) do |a,b,c,d,e| puts "rcv im: #{a}, #{b}, #{c}, #{d}, #{e}" end
    
  end
end

#prpl-jabber
PurpleGW.new.start ARGV[0], ARGV[1], ARGV[2]
