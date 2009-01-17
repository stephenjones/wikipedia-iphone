# rm -r ~/.ruby_inline; ARCHFLAGS="-arch i386" ruby -r rubygems -r bzipread.rb \
# -e 'BzipReader.new.readBlock("../ga.wp.txt.bz2", 0)'

require 'rubygems'
require 'tempfile'
require 'inline'
require File.join(File.dirname(__FILE__), 'common')

BZ_MAX_BLOCK = 1024 * 900 * 5

class BzipReader
  attr :offset
  
  def debug(str)
    $stderr.puts str
  end
  
  inline(:C) do |builder|
    builder.add_compile_flags "-I../c -I. -lbz2"
    builder.add_compile_flags "../c/bzipreader.c"
    builder.add_compile_flags "../c/safe.c"

    builder.prefix '
      #include "bzipreader.h"
      uint64_t readOffset;
    '
    
    ['VALUE __decompressBlock(char *src, int srcLen) {
      char dest[BZ_MAX_BLOCK];
      uint32_t destLen = BZ_MAX_BLOCK;
      int ret;
      
      debug("ruby decompressing %d bytes", srcLen);
      if((ret = decompressBlock(src, srcLen, dest, &destLen)) != BZ_OK)
        fatal("couldn\'t decompress: bz error %d", ret);
        
      return rb_str_new(dest, destLen);
    }',
    
    'VALUE __readBlock(char *file) {
      FILE *in;
      uint64_t realOffset;
      VALUE str;
      
      if(strlen(file) == 0) {
        in = xfopen("/dev/stdin", "rb");
        realOffset = 0;
      } else {
        in = xfopen(file, "rb");
        realOffset = readOffset;
      }        
        
      BitBuffer *bb = bbOfSize(BZ_MAX_BLOCK);
      readOffset = fixedOffset(readBlock(in, realOffset, bb));
      
      xfclose(in);
      
      str = rb_str_new(bb->buff, bb->pos);
      bbClose(bb);
      return str;
    }',
    
    'void __setReadOffset(char *offset) {
      readOffset = *((uint64_t *) offset);
    }',
    
    'VALUE __getReadOffset() {
      return rb_str_new((char *) &readOffset, sizeof(uint64_t));
    }',
    
    'int __computeBoundaries(char *file) {
      int size;
      FILE *in = xfopen(file, "rb");
      size = computeBoundaries(in);
      xfclose(in);
      return size;
    }'].each {|c| builder.c c}
  end
  
  def uint64_to_char(num)
    hi = num >> 32
    lo = num & 0xffffffff
    [lo, hi].pack('L2')
  end
  
  def char_to_uint64(char)
    lo, hi = char.unpack('L2')    
    return lo + (hi << 32)
  end
  
  def initialize(file="")
    if file.empty?
      @useStdin = true
      @buffered = ""
      @eof = false
    end
    
    @file = file
    @offset = 0
  end
  
  def getReadOffset
    char_to_uint64(__getReadOffset)
  end
  
  def setReadOffset(num)
    __setReadOffset(uint64_to_char(num))
  end
  
  def readNextBlock
    # in bzipreader.c, readBlock reads more than it should -- to determine the end of a block,
    # it reads the header for the subsequent block. If we're reading from a file, this is ok;
    # we can seek back to where we want to be. But when reading from stdin, it's more awkward,
    # and we have to buffer things
    # TODO: use a named pipe
    if @useStdin
      if !@buffered or (@offset > 0 and @buffered.size < 40)
        raise EOF
      end
      
      begin
        @buffered += $stdin.read(BZ_MAX_BLOCK) if @buffered.size < BZ_MAX_BLOCK and !@eof
      rescue TypeError
        @eof = true
      end
      
      tempfile = Tempfile.new('bzipreader')
      tempfile.write(@buffered)
      tempfile.flush
      
      setReadOffset(0)
      
      block = __readBlock(tempfile.path)
      
      offset = getReadOffset - 80
      @offset += offset
      bytes = (offset >> 3)
      @buffered = @buffered[bytes..-1]
      
      block
    else
      readBlock(@offset)
    end
  end
  
  def readBlock(offset)
    unless @useStdin
      if File.size(@file) < (offset >> 3) + 80 + 40 # don't ask
        raise EOF
      end
    end
    setReadOffset(offset)
    block = __readBlock(@file)
    @offset = getReadOffset
    block
  end
  
  def decompressBlock(str)
    __decompressBlock(str, str.size)
  end
  
  def computeBoundaries
    __computeBoundaries(@file)
  end
  
  def self.test(skip=0)
    b = BzipReader.new('../ga.wp.txt.bz2')
    skip.times do b.readNextBlock end
    block = b.readNextBlock
    block = b.decompressBlock(block)[0..100]
    puts block
  end
  
  def self.stdin_test(skip = 0)
    b = self.new
    skip.times do b.readNextBlock end
    blockNo = -1
    
    while true
      begin
        offset = b.offset
        block = b.readNextBlock
        plaintext = b.decompressBlock(block)
        $stderr.puts "#{blockNo += 1}\t#{offset}\t#{plaintext.gsub(/\n/, "\\n")[0..30]}"
        $stdout.write plaintext
      rescue EOF
        $stderr.puts "EOF"
        break
      end
    end
  end
end
