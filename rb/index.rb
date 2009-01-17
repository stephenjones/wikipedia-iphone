require File.join(File.dirname(__FILE__), 'bzipreader')
require File.join(File.dirname(__FILE__), 'article')

$nblock = -1
$startblock = 0

def get_line  
  if $lines.empty?
    txt = $reader.decompressBlock($reader.readNextBlock)
    $nblock += 1
    
    $stderr.puts "#{$nblock}\t#{$reader.offset}\t#{txt.gsub(/\n/, "\\n")[0..30]}\t#{txt.size}"
    
    $lines = txt.split("\n").reverse
  end
  
  $lines.pop
end

def quote(str)
  str.gsub(/'/, "''")
end

def process_titles
  while true
    line = get_line
    block = $nblock
    if line.chomp == START_HEADING
      article = get_line.chomp
      puts "#{article}  #{block}" if $startblock <= $nblock
    end
  end
rescue EOF
  $stderr.puts "Done"
end

def main
  if ARGV[1]
    $startblock = ARGV[1].to_i
  end
  
  if ARGV.empty?
    $reader = BzipReader.new
  else
    $reader = BzipReader.new(ARGV.first)
  end
  
  $lines = []
  process_titles
end

if __FILE__ == $0
  main
end
