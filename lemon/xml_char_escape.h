#ifndef XML_CHAR_ESCAPE_H_INCLUDED
#define XML_CHAR_ESCAPE_H_INCLUDED

const char * xml_char_escape(char c)
{
  // I thought raw hex would work in xml, e.g. &x01;, but this
  // isn't bearing up well in practice. E.g. &#xa; is fine, b and c
  // are not, then d is fine again. So that's some character encoding thing
  // not bytes. A lookup table from char to escaped string is thus tricky.
  // Leaning toward encoding anything awkward as a _ab_ hex string inside underscores,
  // where maybe one should toggle into lowercase hex with a _ and then toggle out with
  // a second, escaping _ in the input, such that __ never arises. That would give something
  // mostly readable for identifiers like Thing.bytes? where hex for the ? and maybe the .
  // ideally wouldn't induce hex for the entire string.
  unsigned char u = (unsigned char)c;
  static const char* table[] = {
    [0] = "&#x00",


  };
  return table[u];
}

#endif
