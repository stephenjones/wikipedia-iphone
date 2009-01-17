require 'rubygems'
require 'mongrel'
require 'inline'

READER = "WP, the free offline Wikipedia reader thing"
MAXRES = 40
USAGE = %Q[
<strong>Supported requests:</strong>
<ul>
<li><tt>/wiki/Foo</tt> - parsed Wikipedia article on Foo</li>
<li><tt>/raw/Foo</tt> - raw text of article</li>
<li><tt>/search?s=foo</tt> - article titles containing foo</li>
</ul>
]
SEARCH_BOX=%Q{<div width="100" style="float: right;">
      <form action="/search" method="get">
      <input type="text" name="s" accesskey="s" /></form></div>}

class String
  def titleize
    self[0..0].upcase + self[1..-1]
  end
end

class Parser
  # Parser.php is 5k lines, and has a billion dependencies, transclusion logic,
  # magic-word support, langauge processing, templating code (a literally
  # Turing-complete sublanguage), and stuff... so let's just use regexps
    
  def parse_infobox(str)
    str.gsub(/^\|([^=]+)[:space:]*=[:space:]*(.*)$/) { "<b>#{$1}</b> #{$2}<br>" }
  end

  def parse_template(str)
    str.gsub(/\{\{(([^[:space:]]+) (\w+)$)?([^\}\{]+)\}\}/) { 
    if $2 == "Infobox"
      %Q[ <div style="width: 200px; background-color: #eee; border: 1px solid #bbb; float: right">
      <h2>#{$3}</h2>
      #{parse_infobox($4)} </div> ]
    else
      ""
    end
    }
  end

  def parse(str)
    # Lists; this barely works in even the simplest cases
    str = str.gsub(/^\*(.+)$/) { "<span style=\"display: block\">* #{$1}</span>" }

    # Bold
    str = str.gsub(/'''(.+?)'''/) { "<b>#{$1}</b>" }

    # Italic
    str = str.gsub(/''(.+?)''/) { "<i>#{$1}</i>" }

    # Headings
    str = str.gsub(/(={2,})([^=]+)\1/) {"<h#{$1.size}>#{$2}</h#{$1.size}>"}

    # Interwiki links
    str = str.gsub(/\[\[([^\|\]]+)\|([^\]]+)\]\](\w*)/) { Article.link_to($1, $2 + $3) }
    str = str.gsub(/\[\[(.*?)\]\](\w*)/) { Article.link_to($1, $1 + $2) }

    # Strip refs
    str = str.gsub(/<ref>.*?<\/ref>/, '')

    # Templates, which may be nested, and which we need to parse from innermost first
    while (new = parse_template(str)) != str
      str = new
    end

    # External links
    str = str.gsub(/\[([^\][:space:]]+) ([^\]]+)\]/) {"<a href=\"#{$1}\">#{$2}</a>"}
    str = str.gsub(/\[([^\][:space:]]+)\]/) {"<a href=\"#{$1}\">#{$1}</a>"}
  end
end

class Article
  attr_accessor :text
  attr_accessor :block
  attr_accessor :title

  def parsed_text
    Parser.new.parse(self.text)
  end

  def as_html
    %Q[<html><head><title>#{title} - #{READER}</title></head>
     <body>
      #{SEARCH_BOX}
      <h1>#{title}</h1><h5>#{READER}</h5><p><small>#{text.size} bytes from block #{block}
      (<a href="/raw/#{CGI::escape(title.titleize)}">raw</a>)</small></p>
      #{parsed_text}
     </body></html>]
  end

  def self.link_to(name, text)
    "<a href=\"/wiki/#{CGI::escape(name.titleize)}\">#{text}</a>"
  end
end

class SearchResult
  attr_accessor :results
  attr_accessor :needle

  def as_html
    %Q[<html><head><title>Search: #{needle} - #{READER}</title></head>
    <body>
    #{SEARCH_BOX}
    <h1>Search: #{needle}</h1>
    <ul>#{results.map{|r| "<li>" + Article.link_to(r, r) + "</li>"}.join}</ul>
    <p><small>Searches return up to #{MAXRES} articles containing the search string anywhere in their title.
    Results are case-insensitive. Exact matches appear first, followed by prefix matches, and, lastly, substring matches.
    Press ^S to quickly jump to search.</small></p>
    </body>
    </html>]
  end
end

class WPArticleReader
  inline(:C) do |builder|
    builder.add_compile_flags "-I../c -I. -lbz2 -DDEBUG"
    builder.add_compile_flags "../c/bzipreader.c"
    builder.add_compile_flags "../c/wp.c"
    builder.add_compile_flags "../c/lsearcher.c"
    builder.add_compile_flags "../c/safe.c"
    builder.add_compile_flags "../c/blocks.c"

    builder.prefix %Q$
      #include "wp.h"
      #define MAXRES #{MAXRES}
      #define MAXSTR 1024
      
      wp_dump d = {0};
      wp_article a = {0};

      char results[MAXRES][MAXSTR];
      int nresults;

      bool __handle_result(char *s) {
        strncpy(results[nresults], s, MAXSTR);
        results[nresults][MAXSTR - 1] = \'\\0\';
        char *end = strrchr(results[nresults], \' \');

        if(end) {
          *(end - 1) = \'\\0\';
          nresults++;
        }

        return nresults < MAXRES;
      }
    $

    builder.c 'void __load_dump(char *dump, char *loc, char *ploc, char *blocks) {
      load_dump(&d, dump, loc, ploc, blocks);
      init_article(&a);
    }'

    builder.c 'char *__load_article(char *name) {
      a.block = 0;
      a.text[0] = \'\0\';
      load_article(&d, name, &a);
      return a.text;
    }'

    builder.c 'int __article_block() {
      return a.block;
    }'

    builder.c 'int __article_size() {
      return strlen(a.text);
    }'


    builder.c 'int __search(char *needle) {
      nresults = 0;
      search(&d.index, needle, __handle_result, NULL, true, true);
      return nresults;
    }'

    builder.c 'char *__result(int n) {
      return results[n];
    }'
  end
      
  def initialize(opts)
    @locatedb = opts[:locatedb]
    @prefixdb = opts[:prefixdb]
    @blockdb = opts[:blockdb]
    @dump = opts[:dump]
    
    __load_dump(@dump, @locatedb, @prefixdb, @blockdb)
  end

  def fetch(name)
    text = __load_article(name)
    a = Article.new
    a.text = text
    a.block = __article_block
    a.title = name
    a
  end

  def find(name)
    n = __search(name)
    r = SearchResult.new
    r.needle = name
    r.results = (0..n - 1).map {|n| __result(n)}
    r
  end
end

class WPHandler < Mongrel::HttpHandler
  def initialize(base)
    $stderr.puts "Using base #{base}"
    @reader = WPArticleReader.new(:locatedb => "#{base}locate.db",
                                  :prefixdb => "#{base}locate.prefixdb",
                                  :blockdb => "#{base}blocks.db",
                                  :dump => "#{base}processed")
    @parser = Parser.new 
  end

  def path(req)
    CGI::unescape(req.params["REQUEST_URI"])
  end

  def notfound(resp, str)
    respond(resp, true, 404) { "Couldn't find #{str}. <p>#{USAGE}</p>" }
  end

  def respond(resp, html=true, status=200)
    resp.start(200) do |h, o|
      h["Content-type"] = (html ? 'text/html' : 'text/plain') + '; charset=utf-8'
      o.write yield
    end
  end

  def process(req, resp)
    if path(req) =~ /^\/(wiki|raw)\/(.+)$/
      article = @reader.fetch($2)

      if article.text.empty?
        notfound(resp, $2)
      else
        if $1 == "wiki"
          respond(resp) { article.as_html }
        elsif $1 == "raw"
          respond(resp, false) { article.text }
        end
      end
    elsif path(req) =~ /^\/search\?s=(.+)$/
      respond(resp) { @reader.find($1).as_html }
    else 
      notfound(resp, path(req))
    end
  end
end

class WPServer
  def self.start_on(port)
    self.new(:port => port).run
  end

  def initialize(opts={})
    @port = opts[:port] || 9000
    @host = opts[:host] || '0.0.0.0'
  end

  def run
    $stderr.puts "Binding to #{@host}:#{@port}"

    conf = Mongrel::Configurator.new(:port => @port, 
                                     :host => @host) {
      listener do
        uri '/', :handler => WPHandler.new(ARGV.first), :in_front => true
      end

      trap("INT") { stop }

      run
    }

    conf.join
  end
end
