#include "lexer.h"
#include <stddef.h>

typedef enum { //виды узлов
    NODE_LIST,
    NODE_AND_OR,
    NODE_PIPELINE,
    NODE_SIMPLE,
    NODE_SUBSHELL
} NodeType;

typedef struct { //в файл или из файла
    TokenType op;
    char *file;
} Perenapravlenie;

typedef struct Node Node;

typedef struct {
    Node **items; //массив указателей на дочерние узлы
    char *razdelit; //массив разделителей (символов) между элементами
    size_t count; //количество элементов в списке
} ListData;

typedef struct{
    Node **items;
    TokenType *oper; //массив операторов
    size_t count;
} AndOrData;

typedef struct{
    Node **items;
    size_t count;
} PipelineData;

typedef struct{
    char **words; //массив строк-команд
    Perenapravlenie *peren;
    size_t count; //количество перенаправлений
} SimpleData;

typedef struct {
    Node *body; //вложенный список
    Perenapravlenie *peren;
    size_t count; //количество перенаправлений
} SubshellData;

struct Node {
    NodeType type;
    union {
        ListData list;
        AndOrData and_or;
        PipelineData pipeline;
        SimpleData simple;
        SubshellData subshell;
    } soderzhimoe;
};