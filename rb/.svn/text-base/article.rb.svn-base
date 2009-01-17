# these are actually real ASCII codes
START_HEADING = 1.chr
START_TEXT = 2.chr
END_TEXT = 3.chr

class Article
  attr_accessor :title
  attr_accessor :body

  def body
    @body ||= ''
  end
  
  def write(str)
    str.puts START_HEADING
    str.puts title
    str.puts body.size
    str.puts START_TEXT
    str.puts body
    str.puts END_TEXT
    str.flush
  end
end

def stdin_gets
  raise EOF if $stdin.closed?
  line = $stdin.gets
  raise EOF unless line
  line
end
