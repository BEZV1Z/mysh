#ifndef MYSH_LEXER_H
#define MYSH_LEXER_H
#include <stddef.h>

typedef enum {
    TOKEN_WORD,     // слово: имя команды, аргумент, имя файла 
    TOKEN_PIPE,     // | 
    TOKEN_AND,      // & 
    TOKEN_SEMI,     // ; 
    TOKEN_AND_AND,  // && 
    TOKEN_OR_OR,    // || 
    TOKEN_LPAREN,   // ( 
    TOKEN_RPAREN,   // ) 
    TOKEN_LESS,     // < 
    TOKEN_GREAT,    // > 
    TOKEN_DGREAT,   // >> 
    TOKEN_EOF,      // конец строки достигнут корректно 
    TOKEN_ERROR     // ошибка лексера (например, незакрытая кавычка) 
} TokenType;

typedef struct {
    TokenType type;
    char *text;
} Token;

typedef struct {
    const char *line;
    size_t pos;
} Lexer;

void lexer_init(Lexer *lx, const char *line);
Token lexer_next(Lexer *lx);
void lexer_free_token(Token *tok);

#endif 