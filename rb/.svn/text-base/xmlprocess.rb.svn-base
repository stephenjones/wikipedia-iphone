require "rexml/document"
require "rexml/streamlistener"
require File.join(File.dirname(__FILE__), 'article')

include REXML

class ArticleListener
  include StreamListener
  
  def initialize
    @processed = 0
    @start = Time.now
  end
  
  def text(text)
    @cur_text = text.gsub(/^\[\[[^\]\[]+?\:[^\]\[]+?\]\]$/, '').gsub(/\n+/, "\n")
  end
  
  def print_stats
    rate = (((@processed.to_f / (Time.now - @start)) * 100).round) / 100.0
    $stderr.puts "Processed: #{@processed}\tRate: #{rate}/sec"
  end
  
  def is_desirable(article)
    not (article.title =~ /\:/ or article.title =~ /\//)
  end
  
  def tag_start(name, attrs)
    if name == 'page'
      @cur_article.write($stdout) if (@cur_article and is_desirable(@cur_article))
      @cur_article = Article.new
      @processed += 1
      print_stats if (@processed % 100) == 0
    end
  end
  
  def tag_end(name)
    if name == 'title'
      @cur_article.title = @cur_text
    elsif name == 'text'
      @cur_article.body = @cur_text
    end
  end
end

def process
  Document.parse_stream($stdin, ArticleListener.new)
end
    
if __FILE__ == $0
  process
end
