# XML

This is a thought experiment. What if you build a load of code generation on XML?

It exists because lots of the internal structures of compilers and languages are trees and I noticed [an xslt transform](https://en.wikipedia.org/wiki/XSLT) is a DSL for transforming trees into other trees.

It's rendered in C because I like thinking in C. Your mileage may vary.

The [pragmatic programmer](https://pragprog.com/titles/tpp20/the-pragmatic-programmer-20th-anniversary-edition/) makes a compelling argument for plain text and code generators. @spakhm wrote a [blog post](https://www.defmacro.org/ramblings/lisp.html) roughly equating XML and lisp. I like lisp and I like code generators - therefore what do you get if you use XML instead of plain text to run said code generators?

So far, a tar pit.

It's possible I shouldn't have chosen to script this in [makefiles](makefile). But there are things that show promise:

1. I really like the XML schema premise. Mechanically validating the contents of text files is very like having a static type system for data.
2. [Commonmark to XML](https://github.com/commonmark/cmark) works. [Planning](Planning) turns directories containing markdown into some HTML that renders as horizontally scrolling cards, sort of. The codegen pipeline works, my stubborn refusal to learn CSS is currently limiting. I'm using that to visualise work in progress on various projects.
3. Parser generation. If you write some regex and a CFG grammar in an XML file, you can generate _lots_ from that. It's definitely possible to go from some CFG language written down in an XML file to a large amount of C that will turn an instance of that language into a concrete syntax tree denoted in XML syntax.
4. One can build a [regex to C](regex) compiler in C. [re2c](https://github.com/skvadrik/re2c) is great but it's overkill for the lexers I want to generate.
5. Emacs knows what "relax-ng" schema in "compact format" are, so that is the winner of the competing xml schema languages as far as this project goes. 

Some things are unexpectedly awful:
1. Xsltproc segfaults if you give it [big files](subtransforms/pretty.xsl).
2. XML looks like it can encode binary data. `&#10;` sure looks like a hex escape. It can't, most numbers don't work after the `#`.
3. Writing xslt is far harder than anticipated.
