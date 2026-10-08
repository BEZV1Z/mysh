#define _POSIX_C_SOURCE 200809L

#include "lexer.h"
#include <stdlib.h>
#include <string.h>

#define WORD_BUF_INITIAL_CAP 64 // стартовый размер буфера слова, растёт вдвое через ensure_capacity

static int is_operator_char(char c) // проверка метасимволов
{
    return c == '|' || c == '&' || c == ';' ||
           c == '(' || c == ')' || c == '<' || c == '>';
}

static Token make_token(TokenType type, char *text, const char *error_msg) {
    Token tok;
    tok.type = type;
    tok.text = text;
    tok.error_msg = error_msg;
    return tok;
}

static void ensure_capacity(char **buf, size_t *cap, size_t i) // растим буфер при нехватке места
{
    if (i == *cap) {
        *cap *= 2;
        *buf = realloc(*buf, *cap);
    }
}

void lexer_init(Lexer *lx, const char *line) // обнуление лексера
{
    lx->line = line;
    lx->pos = 0;
}

void lexer_free_token(Token *tok) //обнуление токена
{
    free(tok->text);
    tok->text = NULL;
}

Token lexer_next(Lexer *lx){ // основная функция скана
    while(lx->line[lx->pos] == ' ' || lx->line[lx->pos] == '\t'){
        lx->pos++;
    }
    if (lx->line[lx->pos] == '\0') {
        return make_token(TOKEN_EOF, NULL, NULL);
    }

    switch (lx->line[lx->pos]) {
    case ';':
        lx->pos++;
        return make_token(TOKEN_SEMI, NULL, NULL);
    case '(':
        lx->pos++;
        return make_token(TOKEN_LPAREN, NULL, NULL);
    case ')':
        lx->pos++;
        return make_token(TOKEN_RPAREN, NULL, NULL);
    case '<':
        lx->pos++;
        return make_token(TOKEN_LESS, NULL, NULL);
    case '|':
        lx->pos++;
        if (lx->line[lx->pos] == '|') {
            lx->pos++;
            return make_token(TOKEN_OR_OR, NULL, NULL);
        }
        return make_token(TOKEN_PIPE, NULL, NULL);
    case '&':
        lx->pos++;
        if (lx->line[lx->pos] == '&') {
            lx->pos++;
            return make_token(TOKEN_AND_AND, NULL, NULL);
        }
        return make_token(TOKEN_AND, NULL, NULL);
    case '>':
        lx->pos++;
        if (lx->line[lx->pos] == '>') {
            lx->pos++;
            return make_token(TOKEN_DGREAT, NULL, NULL);
        }
        return make_token(TOKEN_GREAT, NULL, NULL);
    }

    if (lx->line[lx->pos] == '#') { // проверка на коммент
        while (lx->line[lx->pos] != '\0') {
            lx->pos++;
        }
        return make_token(TOKEN_EOF, NULL, NULL);
    }


    size_t cap = WORD_BUF_INITIAL_CAP;
    size_t i = 0;
    char *buf = malloc(cap); // динамический буфер

    while (lx->line[lx->pos] != '\0' && //главная проверка слова
           lx->line[lx->pos] != ' ' &&
           lx->line[lx->pos] != '\t' &&
           !is_operator_char(lx->line[lx->pos])) {
        if (lx->line[lx->pos] == '\'') { // проверка на кавычку
            lx->pos++; // пропуск открывающей
            while (lx->line[lx->pos] != '\'') {
                if (lx->line[lx->pos] == '\0') {
                    free(buf);
                    return make_token(TOKEN_ERROR, NULL, "lexer: незакрытая кавычка '");
                }
                ensure_capacity(&buf, &cap, i);
                buf[i] = lx->line[lx->pos];
                i++;
                lx->pos++;
            }
            lx->pos++; // пропуск закрывающей. В двойной так же
        } else if (lx->line[lx->pos] == '"') {
            lx->pos++;
            while (lx->line[lx->pos] != '"') {
                if (lx->line[lx->pos] == '\0') {
                    free(buf);
                    return make_token(TOKEN_ERROR, NULL, "lexer: незакрытая кавычка \"");
                }
                if (lx->line[lx->pos] == '\\' && lx->line[lx->pos + 1] == '\n') {
                    lx->pos += 2; // '\' и перевод строки исчезают целиком, ничего не копируем
                    continue;
                }
                if (lx->line[lx->pos] == '\\' &&
                    (lx->line[lx->pos + 1] == '"' ||
                     lx->line[lx->pos + 1] == '\\')) {
                    lx->pos++; // пропустили слеш со спецсимволами
                }
                ensure_capacity(&buf, &cap, i);
                buf[i] = lx->line[lx->pos];
                i++;
                lx->pos++;
            }
            lx->pos++;
        } else {
            if (lx->line[lx->pos] == '\\') {
                lx->pos++; //проверка экранирования
                if (lx->line[lx->pos] == '\0') { //проверка на конец строки после слеша
                    free(buf);
                    return make_token(TOKEN_ERROR, NULL, "lexer: висящий '\\' в конце строки");
                }
            }
            ensure_capacity(&buf, &cap, i);
            buf[i] = lx->line[lx->pos];
            i++;
            lx->pos++;
        }
    }
    ensure_capacity(&buf, &cap, i);
    buf[i] = '\0';

    return make_token(TOKEN_WORD, buf, NULL);
}