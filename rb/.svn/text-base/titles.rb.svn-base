require File.join(File.dirname(__FILE__), 'article')
require File.join(File.dirname(__FILE__), 'common')

def process_titles
  while true
    line = stdin_gets
    if line.chomp == START_HEADING
      puts stdin_gets.chomp
    end
  end
rescue EOF
  $stderr.puts "Done"
end

if __FILE__ == $0
  process_titles
end