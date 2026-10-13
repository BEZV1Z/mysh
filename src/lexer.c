#define _POSIX_C_SOURCE 200809L

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_BUF_INITIAL_CAP 64 // стартовый размер буфера слова. растёт вдвое через ensure_capacity

static int is_operator_char(char c) // проверка метасимволов
{
    return c == '|' || c == '&' || c == ';' ||
           c == '(' || c == ')' || c == '<' || c == '>';
}

static Token make_token(TokenType type, char *text) // собирает токен из типа и текста
{
    Token tok;
    tok.type = type;
    tok.text = text;
    return tok;
}

static void lexer_error(const char *msg) // печатает ошибку сразу на месте
{
    fprintf(stderr, "mysh: %s\n", msg);
}

static void ensure_capacity(char **buf, size_t *cap, size_t i) // растим буфер при нехватке места
{
    if (i == *cap) {
        *cap *= 2;
        *buf = realloc(*buf, *cap);
    }
}

static int scan_single_quoted(Lexer *lx, char **buf, size_t *cap, size_t *i) // разбор одинарных кавычек
{
    lx->pos++; // пропуск открывающей кавычки
    while (lx->line[lx->pos] != '\'') {
        if (lx->line[lx->pos] == '\0') {
            lexer_error("lexer: незакрытая кавычка '");
            return -1;
        }
        ensure_capacity(buf, cap, *i);
        (*buf)[*i] = lx->line[lx->pos];
        (*i)++;
        lx->pos++;
    }
    lx->pos++; // пропуск закрывающей кавычки
    return 0;
}

static int scan_double_quoted(Lexer *lx, char **buf, size_t *cap, size_t *i) // разбор двойных кавычек с экранированием
{
    lx->pos++; // пропуск открывающей кавычки
    while (lx->line[lx->pos] != '"') {
        if (lx->line[lx->pos] == '\0') {
            lexer_error("lexer: незакрытая кавычка \"");
            return -1;
        }
        if (lx->line[lx->pos] == '\\' && lx->line[lx->pos + 1] == '\n') {
            lx->pos += 2; // '\' и перевод строки исчезают целиком
            continue;
        }
        if (lx->line[lx->pos] == '\\' &&
            (lx->line[lx->pos + 1] == '"' ||
             lx->line[lx->pos + 1] == '\\')) {
            lx->pos++; // пропустили слеш со спецсимволами
        }
        ensure_capacity(buf, cap, *i);
        (*buf)[*i] = lx->line[lx->pos];
        (*i)++;
        lx->pos++;
    }
    lx->pos++; // пропуск закрывающей кавычки
    return 0;
}

static int scan_plain_char(Lexer *lx, char **buf, size_t *cap, size_t *i) // разбор символа вне кавычек. слеш экранирует следующий
{
    if (lx->line[lx->pos] == '\\') {
        lx->pos++; // проверка экранирования
        if (lx->line[lx->pos] == '\0') { // проверка на конец строки после слеша
            lexer_error("lexer: висящий '\\' в конце строки");
            return -1;
        }
    }
    ensure_capacity(buf, cap, *i);
    (*buf)[*i] = lx->line[lx->pos];
    (*i)++;
    lx->pos++;
    return 0;
}

static Token scan_word(Lexer *lx) // разбор слова целиком, собирает буфер через три режима выше
{
    size_t cap = WORD_BUF_INITIAL_CAP;
    size_t i = 0;
    char *buf = malloc(cap); // динамический буфер

    while (lx->line[lx->pos] != '\0' && //главная проверка слова
           lx->line[lx->pos] != ' ' &&
           lx->line[lx->pos] != '\t' &&
           !is_operator_char(lx->line[lx->pos])) {
        int rc;

        if (lx->line[lx->pos] == '\'') {
            rc = scan_single_quoted(lx, &buf, &cap, &i);
        } else if (lx->line[lx->pos] == '"') {
            rc = scan_double_quoted(lx, &buf, &cap, &i);
        } else {
            rc = scan_plain_char(lx, &buf, &cap, &i);
        }

        if (rc != 0) {
            free(buf);
            return make_token(TOKEN_ERROR, NULL);
        }
    }
    ensure_capacity(&buf, &cap, i);
    buf[i] = '\0';

    return make_token(TOKEN_WORD, buf);
}

static void skip_whitespace(Lexer *lx) // пропуск пробелов и табов
{
    while (lx->line[lx->pos] == ' ' || lx->line[lx->pos] == '\t') {
        lx->pos++;
    }
}

static Token scan_operator(Lexer *lx) // метасимволы
{
    switch (lx->line[lx->pos]) {
    case ';':
        lx->pos++;
        return make_token(TOKEN_SEMI, NULL);
    case '(':
        lx->pos++;
        return make_token(TOKEN_LPAREN, NULL);
    case ')':
        lx->pos++;
        return make_token(TOKEN_RPAREN, NULL);
    case '<':
        lx->pos++;
        return make_token(TOKEN_LESS, NULL);
    case '|':
        lx->pos++;
        if (lx->line[lx->pos] == '|') {
            lx->pos++;
            return make_token(TOKEN_OR_OR, NULL);
        }
        return make_token(TOKEN_PIPE, NULL);
    case '&':
        lx->pos++;
        if (lx->line[lx->pos] == '&') {
            lx->pos++;
            return make_token(TOKEN_AND_AND, NULL);
        }
        return make_token(TOKEN_AND, NULL);
    case '>':
        lx->pos++;
        if (lx->line[lx->pos] == '>') {
            lx->pos++;
            return make_token(TOKEN_DGREAT, NULL);
        }
        return make_token(TOKEN_GREAT, NULL);
    }
    return make_token(TOKEN_ERROR, NULL); // вызывается только для метасимволов
}

static Token scan_comment(Lexer *lx) // пропуск комментария до конца строки
{
    while (lx->line[lx->pos] != '\0') {
        lx->pos++;
    }
    return make_token(TOKEN_EOF, NULL);
}

void lexer_init(Lexer *lx, const char *line) // обнуление лексера
{
    lx->line = line;
    lx->pos = 0;
}

void lexer_free_token(Token *tok) // обнуление токена
{
    free(tok->text);
    tok->text = NULL;
}

Token lexer_next(Lexer *lx) // сама функция проверки. Сделана как диспетчер
{
    skip_whitespace(lx);

    if (lx->line[lx->pos] == '\0') {
        return make_token(TOKEN_EOF, NULL);
    }

    if (is_operator_char(lx->line[lx->pos])) {
        return scan_operator(lx);
    }

    if (lx->line[lx->pos] == '#') { // проверка на коммент
        return scan_comment(lx);
    }

    return scan_word(lx);
}