require 'rubygems'
require 'curses'
require 'inline'

class TernarySearcher
  RESULTS = 10
  
  inline(:C) do |builder|
    builder.include '"ternary.h"'
    builder.add_compile_flags "-I../c -I. -DDEBUG"
    builder.add_compile_flags "../c/searcher.c"
    
    builder.prefix "
    char resultbuf[#{RESULTS}][MAXLINE];
    int haveresults;
    "
    
    builder.c "int handleResult(char *s) {
      strncpy(resultbuf[haveresults++], s, MAXLINE);
      if(haveresults == #{RESULTS}) return false;
      else return true;
    }"
    
    builder.c 'void __init(char *indexFile) {
      load_root(indexFile);
    }'
    
    builder.c 'void __prefixSearch(char *s, int n) {
      haveresults = 0;
      root_search(s, handleResult);
    }'
  end
  
  def initialize(index)
    __init(index)
  end
  
  def prefixSearch(str, n)
    @results = []
    __prefixSearch(str, n)
  end
end

class Searcher
  def initialize
    @needle = ""
    @xap = XapianSearcher.new(ARGV.first)
  end
  
  def refresh
    Curses.clear
    Curses.setpos(0, 0)
    Curses.addstr(@needle)
    draw_matches
    Curses.refresh
  end
  
  def draw_matches
    line = 1
    @matches.each do |match|
      match.draw(line += 1)
    end
  end
  
  def search
    @matches = @xap.matches(@needle).map do |match|
      Match.new(match)
    end
  end
  
  def run
    Curses.init_screen
    Curses.noecho
    Curses.stdscr.keypad(true)
    
    loop do
      char = Curses.getch
      
      if char == 127 # Backspace
        @needle = @needle[0..-2] unless @needle.empty?
      else
        @needle += char.chr
      end
      search
      refresh
    end        
  ensure
    Curses.close_screen
  end
end

class Match
  def initialize(str)
    @string = str
  end
  
  def draw(line)
    Curses.setpos(line, 0)
    Curses.addstr(@string)
  end
end

if $0 == __FILE__
  puts "running..."
  Searcher.new.run
end