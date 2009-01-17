#import "ReadStream.h"

@implementation ReadStream

-(ReadStream *) initWithString: (NSString *) s {
  [self init];
  str = s;
  pos = 0;
  return self;
}

-(unichar) safeCharAtIndex: (int) i {
  return (i < [str length]) ? [str characterAtIndex: i] : 0;
}

-(unichar) peekBack {
  return [self safeCharAtIndex: pos - 1];
}

-(unichar) peek {
  return [self safeCharAtIndex: pos];
}

-(unichar) peekTwo {
  return [self safeCharAtIndex: pos + 1];
}

-(BOOL) nextIs: (unichar) c {
  return [self peek] == c;
}

-(BOOL) nextTwoAre: (unichar) c {
  return [self peek] == c && [self peekTwo] == c;
}

-(BOOL) nextAre: (unichar) c1 and: (unichar) c2 {
  return [self peek] == c1 && [self peekTwo] == c2;
}

-(unichar) read {
  unichar c = [self safeCharAtIndex: pos];
  pos++;
  return c;
}

-(BOOL) isAtEnd {
  return pos == [str length];
}

-(BOOL) isAtStart {
  return pos == 0;
}

-(int) position {
  return pos;
}

-(void) setPosition: (int) i {
  pos = i;
}

-(unichar) expect: (unichar) c {
  unichar r;
  if((r = [self read]) != c) NSLog(@"WARNING: expected %c at position %d, found %c", c, pos - 1, r);
  return r;
}

-(NSString *) readUpto: (unichar) c {
  int found = pos;
  
  while(found != [str length] && [self safeCharAtIndex: found++] != c);
  //if(found == [str length]) return NULL; // not found
  
  found--;
  NSString *s = [str substringWithRange: NSMakeRange(pos, found - pos)];
  pos = found;
  
  return s;
}

-(NSString *) readLineUpto: (unichar) c {
  int found = pos;
  unichar cc;
  
  while(found != [str length]) {
      cc = [self safeCharAtIndex: found];
      if (cc == c || cc == '\n') break;
      found++;
  }

  NSString *s = [str substringWithRange: NSMakeRange(pos, found - pos)];
  pos = found;
  
  return s;
}

/* Special hack to detect format specifiers in tables.
 * No format when "||" or "[[" found (could be link with bar inside)
 * "A real solution probably should do multiple passes, and first, should
 * substitute any syntax which takes precedence over tables, especially the
 * link case above. But the would mean a lot of change. And I have no idea
 * if the result is really better. " - Josef
 */
-(NSString *) readLineUptoSingleBar {
  int found = pos;
  unichar c;

  while(found != [str length]) {
    c = [self safeCharAtIndex: found++];
    if (c == '|') {
      c = [self safeCharAtIndex: found++];
      if (c != '|') {
        found -= 2;
        NSString *s = [str substringWithRange: NSMakeRange(pos, found - pos)];
        pos = found+1;
        return s;
      }
      break;
    }
    if (c == '[') {
      c = [self safeCharAtIndex: found++];
      if (c == '[') break;
    }
    if (c == '\n') break;
  }
  return [NSString string];
}

 
/*-(NSString *) readUptoString: (NSString *) s {
  NSRange range = [str rangeOfString: s options: 0 range: NSMakeRange(pos, [str length] - pos)];
  NSString *text;

  if(range.location != NSNotFound) {
    text = [str substringToIndex: range.location];
    pos = range.location;
  } else {
    text = [str substringFromIndex: pos];
  }

  return text;
}*/

@end
