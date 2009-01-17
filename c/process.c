/* Process Wikipedia XML dumps
 * (c) 2008 Josef Weidendorfer (GPL)
 *
 * Usage:
 * - input on stdin or file name as first parameter
 * - output on stdout
 * - progress on stderr (every 1000 processed articles)
 *
 * Assumptions:
 * - No nested tags inside of <title>...</title> and <text>...</text>
 * - <title> of an article before <text>
 *
 * Processing includes:
 * - Extract title & text of articles (ignore rest of XML)
 * - revert encoding of '&', '<', '>', '"' in XML content
 * - Skip articles with ':' and '/' in title
 * - Remove wiki links [[...]] with ':' in the link target
 * - Shrink runs of '\n' to maximal number of 2
 */

#include "safe.h"

/* Article texts larger than SIZE are always skipped */
#define SIZE 300000
/* Article titles are truncated to TLEN bytes */
#define TLEN 256

char buf[SIZE+1];
char currentTitle[TLEN+1];

enum { tagInvalid = 0, tagTitle, tagText };
char* tagName[] = { "(invalid)", "title", "text" };
int tagLen[] = { 0, 5, 4 };

/* forward decl */
int processText(unsigned char* s, int* l);

// for debugging of XML parsing...
#if 0
void gotTag(int t, char* s, int l)
{
  s[l] = 0;
  debug("T %s (%d): '%s'\n", tagName[t], l, s);
}

void gotContent(int t, char* s, int l)
{
  int i;
  for(i=0;i<l;i++) if (s[i] == '\n') s[i] = ' ';
  s[l] = 0;

  if (l>60) {
    s[30] = 0;
    printf("C %s (%d): '%s...%s'\n", tagName[t], l, s, s+l-30);
  }
  else
    printf("C %s (%d): '%s'\n", tagName[t], l, s);
}
#endif

/* Got content <s> of len <l> inside of tag <t>, starting at
 * global byte position <p> of input (used for error output)
 */
void gotContent(int t, char* s, int l, long long p)
{
  static int count = 0;
  static int skipText = 0;
  int len, ulen;

  s[l] = 0;
  if (t == tagTitle) {
    count++;
    strncpy(currentTitle, s, TLEN);
    return;
  }
  if (t != tagText) return;

  // progress
  if ((count % 1000) == 0)
    fprintf(stderr, "Art. %d (at %lld, len %5d) [skipped %d]: '%s'\n",
	    count, p, l, skipText, currentTitle);

  // skip articles with ':' and '/' in title
  if ((strchr(currentTitle, ':') != 0) ||
      (strchr(currentTitle, '/') != 0))  {
    skipText++;
    return;
  }

  //printf("%s: %d\n%s\n\n", currentTitle, l, s);

  len = l;
  ulen = processText((unsigned char*)s, &len);

  if ((count % 1000) == 0) {
    debug("Art. %d ('%s'): Bytelen %d -> %d, UChars %d\n",
	  count, currentTitle, l, len, ulen);
  }

  printf("%c\n%s\n%d\n%c\n%s\n%c\n",
      1, currentTitle, len, 2, s, 3);
}

/* return number of UTF-8 chars.
 * parameters <s> and <l> are in/out,
 * new text can overwrite old as it is always smaller
 */
int processText(unsigned char* s, int* l)
{
  int unichars = 0;
  unsigned char c, nls = 2;
  unsigned char *ipos, *opos, *end;

  ipos = s;     // input position
  opos = s;     // output position
  end = s + *l; 

  /* Fake start of line to allow for link skipping at article start */
  nls = 2;

  while(ipos < end) {
    c = ipos[0];
    if ((nls>0) && c == '[' && ipos[1] == '[') {
      unsigned char* lpos = ipos+2;
      unsigned char cc;
      int hasCollon = 0;
      while(lpos < end) {
        cc = lpos[0];
        if (cc == ':') hasCollon = 1;
        else if (cc == '[') { hasCollon = 0; break; }
        else if (cc == ']') {
          if (lpos[1] != ']') hasCollon = 0;
          if (hasCollon) {
            // found end of link, which includes collon
            ipos = lpos+2;
            c = ipos[0];
          }
          break;
        }
        lpos++;
      }
      // after skipping, it could be the end of the article
      if (c == 0) break;
    }

    if (c == '\n') {
      nls++;
      ipos++;
      continue;
    }

    if (nls>0) {
      /* reduce to maximum of 2 newlines in a row */
      if (nls>2) nls = 2;

      /* revert faking of newlines at start of article */
      if (opos == s) nls = 0;

      while(nls>0) {
        unichars++;
        *opos = '\n';
        opos++;
        nls--;
      }
    }

    /* UTF-8 chars are simple to count... */
    if (c < 128 || c > 192) unichars++;

    if (ipos > opos) *opos = c;
    ipos++; opos++;
  }

  *opos = 0;
  *l = (int) (opos - s);
  return unichars;
}


/* revert encoding of XML content in-place.
 * returns new content length
 */
int revertXMLEncoding(char* s, int l)
{
  char c;
  char *ipos = s, *opos = s, *end = s + l;

  s[l] = 0;
  while(ipos < end) {
    c = ipos[0];

    if (c == '&') {
      switch(ipos[1]) {
	case 'a':
	  if (ipos[2] == 'm' && ipos[3] == 'p' &&
	      ipos[4] == ';') {
	      // decoded char is the same...
	      ipos += 4;
	  }
	  break;
	  case 'l':
	      if (ipos[2] == 't' && ipos[3] == ';') {
		  c = '<';
		  ipos += 3;
	      }
	      break;
	  case 'g':
	      if (ipos[2] == 't' && ipos[3] == ';') {
		  c = '>';
		  ipos += 3;
	      }
	      break;
	  case 'q':
	      if (ipos[2] == 'u' && ipos[3] == 'o' &&
		  ipos[4] == 't' && ipos[5] == ';') {
		  c = '"';
		  ipos += 5;
	      }
	      break;
	  default:
	      break;
      }
    }

    if (ipos > opos) *opos = c;
    ipos++; opos++;
  }
  *opos = 0;
  return (int) (opos - s);
}


int main(int argc, char* argv[])
{
  long long realpos;
  int bytes_read, overlap, tagID, len;
  char *pos, *found, *end, *content, *tag;
  FILE* fp;

  if (argc > 1)
    fp = xfopen(argv[1], "r");
  else
    fp = stdin;

  content = 0;  // state: not reading content
  tag = 0;      // state: not reading tag attributes
  tagID = tagInvalid;

  realpos = 0;
  overlap = 0;
  pos = buf;

  /* XML content should be passed on as one sequence.
   * Therefore, when end of input chunk is found while reading
   * content, this content is copied to beginning of buffer (with
   * length <overlap>) and further reading is done after that.
   * This limits the length of parsable XML content to size of buffer
   * (large XML content simply will be skipped with a warning).
   */
  while(1) {
    if (overlap) {
      /* copy over content from previous input chunk */
      memcpy(buf, pos, overlap);
    }
    if (feof(fp)) {
      /* append end marker */
      bytes_read = sprintf(buf + overlap, "<<<<                  ");
    }
    else
      bytes_read = fread(buf + overlap, 1, SIZE-overlap, fp);
    end = buf + overlap + bytes_read;
    *end = 0;
    pos = buf;
    overlap = 0;
    realpos += bytes_read;

    while(pos<end) {
      if (tag)
        found = strchr(pos, '>');
      else
        found = strchr(pos, '<');

      if (found == 0) {
        if (content == 0 && tag == 0) {
          break;
        }
        if (pos == buf) {
          fprintf(stderr, "Skipping large content at %lld ('%s')\n",
		  realpos - (end-pos), currentTitle);
          tag = content = 0;
          break;
        }
      }
      if (found == 0 || (found + 15 > end)) {
        if (content == 0 && tag == 0) {
          overlap = end - found;
          pos = found;
        }
        else if (tag) {
          overlap = end - tag;
          pos = tag;
          tag = buf;
        }
        else {
          overlap = end - content;
          pos = content;
          content = buf;
        }
        break;
      }

      pos = found + 1;
      if (tag) {
        //gotTag(tagID, tag, pos - tag -1);
        tag = 0;
        if (*(pos-2) == '/') {
	  char text[1];
	  text[0] = 0;
          gotContent(tagID, text, 0,
              realpos - (end-pos));
          content = 0;
        }
        else
          content = pos;
        continue;
      }
      if (content) {
        if ((pos[0] != '/') ||
            (strncmp(pos+1, tagName[tagID], tagLen[tagID]) != 0) ||
            (pos[tagLen[tagID]+1] != '>')) {

          fprintf(stderr, "Error at %ld: </%s> expected\n",
		  (long)(realpos - (end-pos)), tagName[tagID]);
          exit(1);
        }
	len = pos - content -1;
	len = revertXMLEncoding(content, len);
        gotContent(tagID, content, len, realpos - (end-content));
        content = 0;
        pos += (tagLen[tagID] + 2);
        tagID = tagInvalid;
        continue;
      }
      if (strncmp(pos, "title>", 6) == 0) {
        tagID = tagTitle;
        //gotTag(tagID, pos, 5);
        pos += 6;
        content = pos;
        continue;
      }
      if ((strncmp(pos, "text", 4) == 0) &&
          (pos[4] == ' ' || pos[4] == '>')) {
        tagID = tagText;
        tag = pos;
        pos += 4;
        continue;
      }
      if (strncmp(pos, "<<<", 3) == 0) {
        exit(1);
      }
    }
  }
}











