#include "parser.h"

void parser_init(Parser *p, const char *line) // готовит парсер к разбору строки
{
    lexer_init(&p->lx, line);
    p->current = lexer_next(&p->lx); // сразу читаем первый токен
}

void node_free(Node *node) // освобождает дерево целиком (рекурсивно)
{
    if (node == NULL) {
        return;
    }

    switch (node->type) {
    case NODE_LIST:
        for (size_t i = 0; i < node->soderzhimoe.list.count; i++) {
            node_free(node->soderzhimoe.list.items[i]);
        }
        free(node->soderzhimoe.list.items);
        free(node->soderzhimoe.list.razdelit);
        break;
    case NODE_AND_OR:
        for (size_t i = 0; i < node->soderzhimoe.and_or.count; i++) {
            node_free(node->soderzhimoe.and_or.items[i]);
        }
        free(node->soderzhimoe.and_or.items);
        free(node->soderzhimoe.and_or.oper);
        break;
    case NODE_PIPELINE:
        for (size_t i = 0; i < node->soderzhimoe.pipeline.count; i++) {
            node_free(node->soderzhimoe.pipeline.items[i]);
        }
        free(node->soderzhimoe.pipeline.items);
        break;
    case NODE_SIMPLE:
        for (size_t i = 0; node->soderzhimoe.simple.words[i] != NULL; i++) {
            free(node->soderzhimoe.simple.words[i]);
        }
        free(node->soderzhimoe.simple.words);
        for (size_t i = 0; i < node->soderzhimoe.simple.count; i++) {
            free(node->soderzhimoe.simple.peren[i].file);
        }
        free(node->soderzhimoe.simple.peren);
        break;
    case NODE_SUBSHELL:
        node_free(node->soderzhimoe.subshell.body);
        for (size_t i = 0; i < node->soderzhimoe.subshell.count; i++) {
            free(node->soderzhimoe.subshell.peren[i].file);
        }
        free(node->soderzhimoe.subshell.peren);
        break;
    }

    free(node);
}

static const char *redir_op_str(TokenType op) // строковое представление оператора перенаправления
{
    switch (op) {
    case TOKEN_LESS: return "<";
    case TOKEN_GREAT: return ">";
    case TOKEN_DGREAT: return ">>";
    default: return "?";
    }
}

static void print_indent(int depth) // печатает отступ по глубине вложенности
{
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
}

static void print_peren(const Perenapravlenie *peren, size_t count, int depth) // печатает перенаправления узла
{
    for (size_t i = 0; i < count; i++) {
        print_indent(depth);
        printf("redir %s %s\n", redir_op_str(peren[i].op), peren[i].file);
    }
}

static const char *redir_op_str(TokenType op) // строковое представление оператора перенаправления
{
    switch (op) {
    case TOKEN_LESS: return "<";
    case TOKEN_GREAT: return ">";
    case TOKEN_DGREAT: return ">>";
    default: return "?";
    }
}

static void print_indent(int depth) // печатает отступ по глубине вложенности
{
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
}

static void print_peren(const Perenapravlenie *peren, size_t count, int depth) // печатает перенаправления узла
{
    for (size_t i = 0; i < count; i++) {
        print_indent(depth);
        printf("redir %s %s\n", redir_op_str(peren[i].op), peren[i].file);
    }
}

void node_print(const Node *node, int depth) // печатает дерево с отступами (для дампа)
{
    if (node == NULL) {
        return;
    }

    print_indent(depth);

    switch (node->type) {
    case NODE_LIST:
        printf("LIST\n");
        for (size_t i = 0; i < node->soderzhimoe.list.count; i++) {
            node_print(node->soderzhimoe.list.items[i], depth + 1);
            if (node->soderzhimoe.list.razdelit[i] != '\0') {
                print_indent(depth + 1);
                printf("sep '%c'\n", node->soderzhimoe.list.razdelit[i]);
            }
        }
        break;
    case NODE_AND_OR:
        printf("AND_OR\n");
        for (size_t i = 0; i < node->soderzhimoe.and_or.count; i++) {
            node_print(node->soderzhimoe.and_or.items[i], depth + 1);
            if (i + 1 < node->soderzhimoe.and_or.count) {
                print_indent(depth + 1);
                printf("op %s\n", node->soderzhimoe.and_or.oper[i] == TOKEN_AND_AND ? "&&" : "||");
            }
        }
        break;
    case NODE_PIPELINE:
        printf("PIPELINE\n");
        for (size_t i = 0; i < node->soderzhimoe.pipeline.count; i++) {
            node_print(node->soderzhimoe.pipeline.items[i], depth + 1);
        }
        break;
    case NODE_SIMPLE:
        printf("SIMPLE:");
        for (size_t i = 0; node->soderzhimoe.simple.words[i] != NULL; i++) {
            printf(" %s", node->soderzhimoe.simple.words[i]);
        }
        printf("\n");
        print_peren(node->soderzhimoe.simple.peren, node->soderzhimoe.simple.count, depth + 1);
        break;
    case NODE_SUBSHELL:
        printf("SUBSHELL\n");
        node_print(node->soderzhimoe.subshell.body, depth + 1);
        print_peren(node->soderzhimoe.subshell.peren, node->soderzhimoe.subshell.count, depth + 1);
        break;
    }
}