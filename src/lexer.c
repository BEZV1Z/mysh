#define _POSIX_C_SOURCE 200809L

#include "lexer.h"
#include <stdlib.h>
#include <string.h>

static int is_operator_char(char c) // проверка метасимволов
{
    return c == '|' || c == '&' || c == ';' ||
           c == '(' || c == ')' || c == '<' || c == '>';
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
    if(lx->line[lx->pos] == '\0'){
        Token tok;
        tok.type = TOKEN_EOF;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }

    switch (lx->line[lx->pos]) {
    case ';': {
        Token tok;
        lx->pos++;
        tok.type = TOKEN_SEMI;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }
    case '(': {
        Token tok;
        lx->pos++;
        tok.type = TOKEN_LPAREN;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }
    case ')': {
        Token tok;
        lx->pos++;
        tok.type = TOKEN_RPAREN;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }
    case '<': {
        Token tok;
        lx->pos++;
        tok.type = TOKEN_LESS;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }
    case '|': {
        Token tok;
        lx->pos++;
        if (lx->line[lx->pos] == '|') {
            lx->pos++;
            tok.type = TOKEN_OR_OR;
            tok.text = NULL;
            tok.error_msg = NULL;
            return tok;
        }
        tok.type = TOKEN_PIPE;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }
    case '&': {
        Token tok;
        lx->pos++;
        if (lx->line[lx->pos] == '&') {
            lx->pos++;
            tok.type = TOKEN_AND_AND;
            tok.text = NULL;
            tok.error_msg = NULL;
            return tok;
        }
        tok.type = TOKEN_AND;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }
    case '>': {
        Token tok;
        lx->pos++;
        if (lx->line[lx->pos] == '>') {
            lx->pos++;
            tok.type = TOKEN_DGREAT;
            tok.text = NULL;
            tok.error_msg = NULL;
            return tok;
        }
        tok.type = TOKEN_GREAT;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }
}

    if (lx->line[lx->pos] == '#') { // проверка на коммент
        while (lx->line[lx->pos] != '\0') {
            lx->pos++;
        }
        Token tok;
        tok.type = TOKEN_EOF;
        tok.text = NULL;
        tok.error_msg = NULL;
        return tok;
    }

    
    size_t cap = 64;
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
                    Token tok;
                    tok.type = TOKEN_ERROR;
                    tok.text = NULL;
                    tok.error_msg = "lexer: незакрытая кавычка '";
                    return tok;
                }
                if (i == cap) {
                    cap *= 2;
                    buf = realloc(buf, cap); // увеличили если надо
                }
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
                    Token tok;
                    tok.type = TOKEN_ERROR;
                    tok.text = NULL;
                    tok.error_msg = "lexer: незакрытая кавычка \"";
                    return tok;
                }
                if (lx->line[lx->pos] == '\\' &&
                    (lx->line[lx->pos + 1] == '"' ||
                     lx->line[lx->pos + 1] == '\\' ||
                     lx->line[lx->pos + 1] == '\n')) {
                    lx->pos++; // пропустили слеш со спецсимволами 
                }
                if (i == cap) {
                    cap *= 2;
                    buf = realloc(buf, cap);
                }
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
                    Token tok;
                    tok.type = TOKEN_ERROR;
                    tok.text = NULL;
                    tok.error_msg = "lexer: висящий '\\' в конце строки";
                    return tok;
                }
            }
            if (i == cap) {
                cap *= 2;
                buf = realloc(buf, cap);
            }
            buf[i] = lx->line[lx->pos];
            i++;
            lx->pos++;
        }
    }
    if (i == cap) {
        cap *= 2;
        buf = realloc(buf, cap);
    }
    buf[i] = '\0';

    Token tok;
    tok.type = TOKEN_WORD;
    tok.text = buf; // буфер переходит токену, копия не нужна 
    tok.error_msg = NULL;
    return tok;

}
