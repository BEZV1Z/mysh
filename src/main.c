#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

static const char *type_name(TokenType t) // имя типа токена для вывода
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

static void dump(const char *line) // прогоняет одну строку через лексер и печатает все её токены
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

static void dump_tree(const char *line) // прогоняет строку через парсер и печатает дерево
{
    printf("-- дерево --\n");
    Parser p;
    parser_init(&p, line);
    Node *tree = parser_parse(&p);
    node_print(tree, 0);
    node_free(tree);
}

static char *read_line(FILE *stream) // читает строку произвольной длины из stream, свой аналог getline без POSIX
{
    size_t cap = 128, len = 0;
    char *buf = malloc(cap);

    int c;
    while ((c = fgetc(stream)) != EOF && c != '\n') {
        if (len + 1 == cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }
        buf[len++] = (char)c;
    }

    if (c == EOF && len == 0) {
        free(buf);
        return NULL; // ввод кончился, и ничего не успели прочитать
    }

    buf[len] = '\0';
    return buf;
}

static void run_interactively(void) // читает строки любой длины с stdin, дампит токены и дерево каждой
{
    printf("Вводи строки для разбора, одну за одной (конец ввода — Ctrl+D или Ctrl+Z):\n");

    char *line;
    while ((line = read_line(stdin)) != NULL) {
        dump(line);
        dump_tree(line);
        free(line);
    }
}

int main(void)
{
    run_interactively();
    return 0;
}