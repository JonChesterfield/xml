#include "types.h"
#include "parse.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char ** argv)
{
    int value;
    void* pParser;
    const char *c;
    size_t i = 0;
    struct SToken v[argc];

    if (2 > argc)
    {
        printf("Usage: %s <expression>\n", argv[0]);
        return 1;
    }

    pParser = (void *) ParseAlloc(malloc);
    for (i = 1; i < argc; ++i)
    {
        c = argv[i];
        v[i].token = c;
        switch (*c)
        {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                for (value = 0; *c && *c >= '0' && *c <= '9'; c++)
                    value = value * 10 + (*c - '0');
                v[i].value = value;
                token tok =
                  {.name = TOKEN_ID_INTEGER,
                   .value_start = c,
                   .value_end = c + strlen(c),
                  };
                Parse(pParser, TOKEN_ID_INTEGER, &tok);
                break;

            case '+':
                Parse(pParser, TOKEN_ID_PLUS, NULL);
                break;

            case '-':
                Parse(pParser, TOKEN_ID_MINUS, NULL);
                break;

            case '*':
                Parse(pParser, TOKEN_ID_TIMES, NULL);
                break;

            case '/':
                Parse(pParser, TOKEN_ID_DIVIDE, NULL);
                break;

            case '(':
                Parse(pParser, TOKEN_ID_LPAR, NULL);
                break;

            case ')':
                Parse(pParser, TOKEN_ID_RPAR, NULL);
                break;

            default:
                fprintf(stderr, "Unexpected token %s\n", c);
        }
    }
    Parse(pParser, 0, NULL);
    ParseFree(pParser, free);

    return 0;
}
