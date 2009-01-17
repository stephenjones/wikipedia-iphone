#import "WPParser.h"

@implementation WPParser 

-(WPParser *) initWithMarkup: (NSString *) markup {
  [self init];
  raw = [[ReadStream alloc] initWithString: markup];
  return self;
}

-(void) parseLink {
  NSString *linkText, *linkHref, *text;
  int pos;
  
  [raw expect: '['];
  [raw expect: '[']; 

  /* if we're already in a complex link (like an image), we want to ignore the contents */
  if(linkLevel > 0) {
    linkLevel++;
    return;
  }

  /* so we can reset later */
  pos = [raw position];
  text = [raw readUpto: ']'];
  [raw expect: ']'];
  [raw expect: ']'];

  /* we might want to detect other special links here */
  NSRange imageNS = [text rangeOfString: @"Image:"];
  if(imageNS.location == 0) {
    linkLevel++;
    /* parse again from the start -- from [[.
     * The ]] we found as the end isn't reliable. */
    [raw setPosition: pos];
    return;
  }

  NSRange range = [text rangeOfString: @"|"];
  if(range.location != NSNotFound) {
    linkText = [text substringFromIndex: range.location + 1];
    linkHref = [text substringToIndex: range.location];
  } else
    linkText = linkHref = text;
  
  [html appendFormat: @"<a href=\"wp://localhost/%@\">%@</a>", linkHref, linkText];
}

-(void) parseLinkEnd {
  [raw expect: ']'];
  [raw expect: ']'];
  linkLevel--;
}

-(void) parseExternalLink {
  NSString *linkText, *link;

  [raw expect: '['];

  link = [raw readUpto: ' '];
  linkText = [raw readUpto: ']'];

  [raw expect: ']'];

  /* TODO: figure out how to launch external URLs with Safari */
  /* [html appendFormat: @"<a href=\"%@\">%@</a>", link, linkText]; */
  [html appendFormat: @"<span class=\"external\">%@</span>", linkText];
}

-(void) parseHeading {
  NSString *text;

  int level = 0;
  while([raw peek] == '=') { [raw read]; level++; }
  text = [raw readUpto: '='];
  while([raw peek] == '=') [raw read];
    
  [html appendFormat: @"<h%d>%@</h%d>", level, text, level];
} 

-(void) parseItalic {
  if(italic) {
    italic = NO;
    [html appendString: @"</i>"];
  } else {  
    italic = YES;
    [html appendString: @"<i>"];
  } 
}

-(void) parseBold {
  [raw expect: '\''];

  if(bold) {
    bold = NO;
    [html appendString: @"</b>"];
  } else { 
    bold = YES;
    [html appendString: @"<b>"];
  }
}

-(void) parseBoldItalic {
  [raw expect: '\''];
  [raw expect: '\''];

  if([raw peek] == '\'') [self parseBold];
  else [self parseItalic];
}    

-(void) parseTemplateStart {
  [raw expect: '{'];
  [raw expect: '{'];
  tmplLevel++;
}

-(void) parseTemplateEnd {
  [raw expect: '}'];
  [raw expect: '}'];
  tmplLevel--;
}

-(void) resetIndent: (int) level {
  while(level < [indents count]) {
    [html appendFormat: @"</%@>", [indents objectAtIndex: [indents count] - 1]];
    [indents removeLastObject];
  }
}

-(void) parseIndent {
  int level = 0;
  unichar c = 0;

  /* figure out correct indentation level */
  while([raw peek] == '*' || [raw peek] == '#' || \
        [raw peek] == ':' || [raw peek] == ';') {
    c = [raw read];
    if(c == ';') continue;
    level++;
    if(level > [indents count]) {
      [html appendString: ((c == '#') ? @"<ol>" : @"<ul>")];
      [indents addObject: ((c == '#') ? @"ol" : @"ul")];
    }
  }

  [self resetIndent: level];

  /* <li> placeholder unless we've just finished the list */
  if(c == '*' || c == '#') 
    [html appendString: @"<li>"];
  else
    [html appendString: @"<p>"];

  if(c == ';')  {
    NSString* text = [raw readLineUpto: ':'];
    [html appendFormat: @"<b>%@</b><p>", text];
    if([raw peek] == ':') {
     [html appendString: @"<ul>"];
     [indents addObject: @"ul"];
    }
  }
}

-(void) parseTableBegin {
  NSString *format;

  [raw expect: '{'];
  [raw expect: '|'];

  format = [raw readUpto: '\n'];
  [html appendFormat: @"<p><table %@>", format];

  inTable = YES;
  inTableCaption = NO;
  inTableRow = NO;
  inTableCell = NO;
}

-(void) tableCellEnd {
  if(!inTableCell) return;

  if(inTableHeadingCell)
    [html appendString: @"</th>"];
  else
    [html appendString: @"</td>"];
  inTableCell = NO;
}

-(void) tableCaptionEnd {
  if(inTableCaption) {
    [html appendString: @"</caption>"];
    inTableCaption = NO;
  }
}

-(void) tableRowEnd {
  [self tableCellEnd];
  if(inTableRow) {
    [html appendString: @"</tr>"];
    inTableRow = NO;
  }
}

-(void) parseTableEnd {
  [raw expect: '|'];
  [raw expect: '}'];

  [self tableCaptionEnd];
  [self tableRowEnd];
  [html appendString: @"</table><p>"];

  inTable = NO;
}

-(void) parseTableCaption {
  NSString *format;

  [raw expect: '|'];
  [raw expect: '+'];
  [self tableRowEnd];

  format = [raw readLineUptoSingleBar];
  [html appendFormat: @"<caption %@>", format];
  inTableCaption = YES;
  inTableRow = NO;
  inTableCell = NO;
}

-(void) parseTableRow {
  NSString *format;

  [raw expect: '|'];
  [raw expect: '-'];

  [self tableCaptionEnd];
  [self tableRowEnd];

  format = [raw readUpto: '\n'];
  [html appendFormat: @"<tr %@>", format];
  inTableCaption = NO;
  inTableRow = YES;
  inTableCell = NO;
}

-(void) ensureTableRow {
  if(inTableRow) return;

  [self tableCaptionEnd];
  [html appendString: @"<tr>"];
  inTableRow = YES;
}

-(void) parseTableCell {
  NSString *format;

  [raw expect: '|'];
  [self tableCellEnd];

  [self ensureTableRow];
  format = [raw readLineUptoSingleBar];
  [html appendFormat: @"<td %@>", format];
  inTableCell = YES;
  inTableHeadingCell = NO;
}

-(void) parseTableHeadingCell {
  NSString *format;

  [raw expect: '!'];
  [self tableCellEnd];

  [self ensureTableRow];
  format = [raw readLineUptoSingleBar];
  [html appendFormat: @"<th %@>", format];
  inTableCell = YES;
  inTableHeadingCell = YES;
}

-(void) parseTableDispatch {
  if([raw peek] == '{' && [raw peekTwo] == '|')
    [self parseTableBegin];
  else if(inTable && [raw peek] == '|') {
    if([raw peekTwo] == '-')
      [self parseTableRow];
    else if([raw peekTwo] == '}')
      [self parseTableEnd];
    else if([raw peekTwo] == '+')
      [self parseTableCaption];
    else
      [self parseTableCell];
  } else if(inTable && [raw peek] == '!')
    [self parseTableHeadingCell];
}

-(void) parseLineStart {
  if (!inTable) {
    if([raw peek] == '*' || [raw peek] == '#' || \
       [raw peek] == ':' || [raw peek] == ';')
      [self parseIndent];
    else
      [self resetIndent: 0];
  }

  if([raw peek] == '=')
    [self parseHeading];
  else
    [self parseTableDispatch];
}

-(void) rawRead {
  [html appendFormat: @"%C", [raw read]];
}

-(void) parse {
  while(![raw isAtEnd]) {
    unichar c = [raw peek];

    if(inTable && [raw nextTwoAre: '|']) {
      [raw read]; /* just one | for rawTableCell */
      [self parseTableCell];
    } else if(inTable && [raw nextTwoAre: '!']) {
      [raw read];
      [self parseTableHeadingCell];
    } else if([raw nextTwoAre: '{'])
      [self parseTemplateStart];
    else if([raw nextTwoAre: '}'])
      [self parseTemplateEnd];
    else if(tmplLevel > 0)
      [raw read];

    else if([raw nextTwoAre: ']'])
      [self parseLinkEnd];
    else if([raw nextTwoAre: '['])
      [self parseLink];
    else if(linkLevel > 0)
      [raw read];

    else if([raw nextIs: '['])
      [self parseExternalLink];
    else if([raw nextTwoAre: '\''])
      [self parseBoldItalic];
    else if([raw nextIs: '\n']) {
      [self rawRead]; /* emit the \n */
      if([raw nextIs: '\n']) {
	/* reset bold/italic on new paragraph */
	if (italic) { italic = NO; [html appendString: @"</i>"]; }
	if (bold) { bold = NO; [html appendString: @"</b>"]; }
	[html appendString: @"<p>"];
      }
      [self parseLineStart];
    } else {
      if([raw isAtStart]) {
        [self parseLineStart];
        if([raw isAtStart]) [self rawRead]; /* parseLineStart did nothing */
      } else
        [self rawRead];
    }
  }
}

-(NSString *) parsed {
  html = [[NSMutableString alloc] init];
  indents = [[NSMutableArray alloc] init];
  bold = NO;
  italic = NO;
  tmplLevel = 0;
  linkLevel = 0;
  inTable = NO;
  [self parse];
  return html;
}

@end
