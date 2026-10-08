#include "lexer.h"
#include <stdio.h>

static const char *type_name(TokenType t)
{
    switch (t) {
    case TOKEN_WORD: return "WORD";
    case TOKEN_PIPE: return "PIPE";
    case TOKEN_AND: return "AND";
    case TOKEN_SEMI: return "SEMI";
    case TOKEN_AND_AND: return "AND_AND";
    case TOKEN_OR_OR: return "OR_OR";
    case TOKEN_LPAREN: return "LPAREN";
    case TOKEN_RPAREN: return "RPAREN";
    case TOKEN_LESS: return "LESS";
    case TOKEN_GREAT: return "GREAT";
    case TOKEN_DGREAT: return "DGREAT";
    case TOKEN_EOF: return "EOF";
    case TOKEN_ERROR: return "ERROR";
    }
    return "?";
}

static void dump(const char *line) //дамп проверка работы лексера
{
    printf("== %s ==\n", line);
    Lexer lx;
    lexer_init(&lx, line);
    for (;;) {
        Token t = lexer_next(&lx);
        if (t.type == TOKEN_WORD) {
            printf("  WORD(%s)\n", t.text);
        } else {
            printf("  %s\n", type_name(t.type));
        }
        int done = (t.type == TOKEN_EOF || t.type == TOKEN_ERROR);
        lexer_free_token(&t);
        if (done) break;
    }
}

int main(void)
{
    dump("echo hi | wc");
    dump("a&&b||c;d");
    dump("echo a;b");
    dump("");
    dump("   ");
    dump("a\\;b");
    dump("a\\ b");
    dump("a\\\\b");
    dump("a\\");
    dump("echo 'a b'");
    dump("'ab' c");
    dump("'ab'cd");
    dump("'ab'");
    dump("echo 'unterminated");
    dump("\"a\\tb\"");
    dump("\"a\\\"b\"");
    dump("\"a\\\\b\"");
    dump("\"a b\" c");
    dump("echo \"unterminated");
    dump("echo hi # comment");
    dump("#comment");
    dump("a#b");
    dump("''");
    dump("\"\"");
    dump("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    dump("\"a\\\nb\""); // '\' + реальный перевод строки внутри
    dump("anfn|sdf;sd"); // метасимволы вплотную к словам, без пробелов
    dump("a'b'\"c\"d"); // смешение одинарных и двойных кавычек в одном слове
    return 0;
}