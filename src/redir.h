#ifndef MYSH_REDIR_H
#define MYSH_REDIR_H

#include "parser.h" // отсюда берём Perenapravlenie

int apply_redirection(const Perenapravlenie *r); // открывает файл и подключает его через dup2

#endif 