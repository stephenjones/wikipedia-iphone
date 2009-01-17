require 'xapian'

db = Xapian::WritableDatabase.new(ARGV[0], Xapian::DB_CREATE_OR_OPEN)
stem = Xapian::TermGenerator.new()
f = File.open(ARGV[1], 'r')
processed = 0

begin
  while (line = f.readline)
    begin
      split = line.split("|")
      next if split.first == ""
    
      doc = Xapian::Document.new
      doc.data = line
      doc.add_posting(split.first.downcase, 1)
      db.add_document(doc)
      processed += 1

      if processed % 100 == 0
        $stderr.puts "#{processed}\t#{split.first}"
      end
    rescue
      puts line
      raise $!
    end
  end
rescue EOFError
  $stderr.puts "Done"
end
