#include "redir.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int apply_redirection(const Perenapravlenie *r){ // применяет один редирект: open + dup2 + close
    int flags;
    int target;

    switch (r->op) {
    case TOKEN_LESS:
        flags = O_RDONLY; //файл открываем только на чтение
        target = 0; // stdin
        break;
    case TOKEN_GREAT:
        flags = O_WRONLY | O_CREAT | O_TRUNC; //файл открываем только на чтение / если файла нет, создать / если файл есть, обнулить его перед записью
        target = 1; // stdout
        break;
    case TOKEN_DGREAT:
        flags = O_WRONLY | O_CREAT | O_APPEND; //то же самое, но аппенд писать в конец файла, не стирая то, что там уже было
        target = 1;
        break;
    default: // страховка
        fprintf(stderr, "mysh: неизвестный оператор редиректа\n");
        return -1;
    }

    int fd = open(r->file, flags, 0644); // 0644 нужен только для O_CREAT это права доступа, которые проставятся, если файл будет реально создан
    if (fd == -1) { //если опен не открыл файл то ошибка
        perror("mysh: open");
        return -1;
    }

    if (dup2(fd, target) == -1) { //дап2 заставляет указывать дескрптор таргет(0 или 1) туда же куда и фд
        perror("mysh: dup2");
        close(fd);
        return -1;
    }

    close(fd); // dup2 уже сделал копию на target, сам fd больше не нужен
    return 0;
}