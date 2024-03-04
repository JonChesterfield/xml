local str = [[
<?xml version="1.0" encoding="UTF-8"?>
<Language>
  <LanguageName>regex</LanguageName>
  <Lexer>
    <!-- Currently posix, re2, re2c or multi -->
    <!-- considering lex and the posix from musl -->
    <!-- Multi is set up to run all the engines that lexer.h says are available -->    
    <Engine value="re2c" />
  </Lexer>

  <Tokens>]]

for i = 0, 255 do
   str = string.format([[%s
    <Token name="BYTE%02x" literal="%02x" />]], str, i, i)
end

str = str .. [[


    <Token name="LPAREN" literal="(" />
    <Token name="RPAREN" literal=")" />

    <!--
    a slightly different lexer could do something like:
    <Token name="LSQUARE" literal="S" />
    to create some of the same tokens from different text
    Maybe worth keeping (* as two tokens instead of folding them
    for fractionally longer names in exchange for easier conversion
    Or regex instead of literals, though that'll carry a cost
    <Token name="LSQUARE" regex="\[|S" />
    -->

    <Token name="LSQUARE" literal="[" />
    <Token name="RSQUARE" literal="]" />

    <Token name="EMPTYSET" literal="%" />
    <Token name="EMPTYSTRING" literal="_" />

    <Token name="CONCAT" literal=":" />

    <Token name="KLEENE" literal="*" />

    <Token name="OR" literal="|" />
    <Token name="AND" literal="&amp;" />
    <Token name="NOT" literal="~" />

    <Token name="WHITESPACE" regex="[ \f\n\r\t\v]+" />
  </Tokens>
]]

str = str .. [[

  <Groupings>
    <Grouping name="regex" type="ptree" />

    <Grouping name="empty_set" type="ptree" />
    <Grouping name="empty_string" type="ptree" />

    <Grouping name="concat" type="ptree" />
    <Grouping name="kleene" type="ptree" />
    <Grouping name="or" type="ptree" />
    <Grouping name="and" type="ptree" />
    <Grouping name="not" type="ptree" />
    <Grouping name="range" type="ptree" />
]]

for i = 0, 255 do
   str = string.format([[%s
    <Grouping name="byte_%02x" type="ptree" />]], str, i)
end

str = str .. [[

  </Groupings>

  <Precedences>
  </Precedences>

  <Productions>]]

str = str .. [[

    <AssignProduction label="result" grouping="program" >
      <Grouping type="regex" position="1" />
    </AssignProduction>

    <ListProduction label="make_empty_set" grouping="empty_set" >
      <Token name="EMPTYSET" />
    </ListProduction>

    <!-- Either one turns the empty set into a regex, in which case no further handling is needed:

    or one can keep it as a separate entity and pattern match the various combinations.
    Can't do both as that makes the LALR grammar ambiguous.    
    -->

    <AssignProduction label="from_empty_set" grouping="regex" >
      <Grouping type="empty_set" position="1" />
    </AssignProduction>

<!--
    <AssignProduction label="make_or_left_empty" grouping="regex" >
      <Token name="LPAREN" />
      <Token name="OR" />
      <Grouping type="empty_set" />
      <Grouping type="regex" position="1" />
      <Token name="RPAREN" />
    </AssignProduction>

    <AssignProduction label="make_or_right_empty" grouping="regex" >
      <Token name="LPAREN" />
      <Token name="OR" />
      <Grouping type="regex" position="1" />
      <Grouping type="empty_set" />
      <Token name="RPAREN" />
    </AssignProduction>

    <AssignProduction label="make_and_left_empty" grouping="regex" >
      <Token name="LPAREN" />
      <Token name="AND" />
      <Grouping type="empty_set" position="1" />
      <Grouping type="regex" />
      <Token name="RPAREN" />
    </AssignProduction>

    <AssignProduction label="make_and_right_empty" grouping="regex" >
      <Token name="LPAREN" />
      <Token name="AND" />
      <Grouping type="regex" />
      <Grouping type="empty_set" position="1" />
      <Token name="RPAREN" />
    </AssignProduction>

    <AssignProduction label="make_concat_left_empty" grouping="regex" >
      <Token name="LPAREN" />
      <Token name="CONCAT" />
      <Grouping type="empty_set" position="1" />
      <Grouping type="regex" />
      <Token name="RPAREN" />
    </AssignProduction>

    <AssignProduction label="make_concat_right_empty" grouping="regex" >
      <Token name="LPAREN" />
      <Token name="CONCAT" />
      <Grouping type="regex" />
      <Grouping type="empty_set" position="1" />
      <Token name="RPAREN" />
    </AssignProduction>

    <AssignProduction label="make_kleene_empty" grouping="empty_string" >
      <Token name="LPAREN" />
      <Token name="KLEENE" />
      <Grouping type="empty_set" position="1" />
      <Token name="RPAREN" />
    </AssignProduction>
    
    <ListProduction label="make_not_empty" grouping="not" >
      <Token name="LPAREN" />
      <Token name="NOT" />
      <Grouping type="empty_set" position="1" />
      <Token name="RPAREN" />
    </ListProduction>
-->

    <ListProduction label="make_empty_string" grouping="empty_string" >
      <Token name="EMPTYSTRING" />
    </ListProduction>
    <AssignProduction label="from_empty_string" grouping="regex" >
      <Grouping type="empty_string" position="1" />
    </AssignProduction>

    <ListProduction label="make_concat" grouping="concat" >
      <Token name="LPAREN" />
      <Token name="CONCAT" />
      <Grouping type="regex" position="1" />
      <Grouping type="regex" position="2" />
      <Token name="RPAREN" />
    </ListProduction>
    <AssignProduction label="from_concat" grouping="regex" >
      <Grouping type="concat" position="1" />
    </AssignProduction>

    <ListProduction label="make_kleene" grouping="kleene" >
      <Token name="LPAREN" />
      <Token name="KLEENE" />
      <Grouping type="regex" position="1" />
      <Token name="RPAREN" />
    </ListProduction>
    <AssignProduction label="from_kleene" grouping="regex" >
      <Grouping type="kleene" position="1" />
    </AssignProduction>

    <ListProduction label="make_or" grouping="or" >
      <Token name="LPAREN" />
      <Token name="OR" />
      <Grouping type="regex" position="1" />
      <Grouping type="regex" position="2" />
      <Token name="RPAREN" />
    </ListProduction>

    <AssignProduction label="from_or" grouping="regex" >
      <Grouping type="or" position="1" />
    </AssignProduction>

    <ListProduction label="make_and" grouping="and" >
      <Token name="LPAREN" />
      <Token name="AND" />
      <Grouping type="regex" position="1" />
      <Grouping type="regex" position="2" />
      <Token name="RPAREN" />
    </ListProduction>
    <AssignProduction label="from_and" grouping="regex" >
      <Grouping type="and" position="1" />
    </AssignProduction>

    <ListProduction label="make_not" grouping="not" >
      <Token name="LPAREN" />
      <Token name="NOT" />
      <Grouping type="regex" position="1" />
      <Token name="RPAREN" />
    </ListProduction>
    <AssignProduction label="from_not" grouping="regex" >
      <Grouping type="not" position="1" />
    </AssignProduction>

<!--
    <AssignProduction label="DiscardWSLeft" grouping="regex" >
      <Token name="WHITESPACE" />
      <Grouping type="regex" position="1" />
    </AssignProduction>
-->

    <AssignProduction label="DiscardWSRight" grouping="regex" >
      <Grouping type="regex" position="1" />
      <Token name="WHITESPACE" />
    </AssignProduction>

]]


for i = 0, 255 do
str = str .. string.format([[
    <ListProduction label="make_byte_%02x" grouping="byte_%02x" >
      <Token name="BYTE%02x" />
    </ListProduction>
    <AssignProduction label="from_byte_%02x" grouping="regex" >
      <Grouping type="byte_%02x" position="1" />
    </AssignProduction>

]], i, i, i, i, i)
end

str = str .. [[
  </Productions>
</Language>
]]

print(str)
