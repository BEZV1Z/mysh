#include "executor.h"
#include "redir.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int execute_simple(Node *node) // форкает и запускает одну внешнюю команду
{
    fflush(stdout); // чтобы буфер шелла не потерялся/не задвоился при fork

    pid_t pid = fork();
    if (pid == -1) {
        perror("mysh: fork");
        return 1;
    }

    if (pid == 0) {// ребёнок
        for (size_t i = 0; i < node->soderzhimoe.simple.count; i++) {
            if (apply_redirection(&node->soderzhimoe.simple.peren[i]) != 0) {
                _exit(1); //если какой-то редирект не удался - выход
            }
        }
        execvp(node->soderzhimoe.simple.words[0], node->soderzhimoe.simple.words);
        perror("mysh: execvp"); // сюда попадаем только если execvp не смогла запуститься
        _exit(127); // команда не найдена
    }

    // родитель
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return 1; // убит сигналом или что-то странное
}

int execute(Node *node) // выполняет узел дерева, возвращает код возврата
{
    if (node == NULL) {
        return 0;
    }

    switch (node->type) {
    case NODE_SIMPLE:
        return execute_simple(node);
    default:
        fprintf(stderr, "mysh: этот тип узла пока не умеем выполнять\n");
        return 1;
    }
}