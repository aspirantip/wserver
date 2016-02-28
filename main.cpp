#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>



int main(int argc, char *argv[])
{
    std::cout << "run application ...\n";

    // если запускаем без аргументов, выводим справку
    if(argc == 1) {
        printf("usage:\n");
        printf(" ./wserver -h <ip> -p <port> -d <directory>\n");
        printf("example:\n");
        printf(" $ ./wserver -h '127.0.0.1' -p 12345 -d '/tmp/test/'\n");
        return 0;
    }

    char *opts = "h:p:d:";          // доступные опции, каждая принимает аргумент
    int port;                       // тут храним числа
    char* host;                     // а тут оператор
    char* dir;                      // директория

    int opt;                        // каждая следующая опция попадает сюда
    while((opt = getopt(argc, argv, opts)) != -1) { // вызываем getopt пока она не вернет -1
        switch(opt) {
        case 'h':
            host = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'd':
            dir = optarg;
            break;
        }
    }

    if (1) {                    // print parameters
        printf("parametrs:\n");
        printf("\t host = %s\n", host);
        printf("\t port = %d\n", port);
        printf("\t dir  = %s\n", dir);
    }


    pid_t par_pid = getpid();
    printf("Parant PID = %d\n", par_pid);

    pid_t ch_pid = fork();
    if( !ch_pid )
    {
        printf("Child PID = %i (child process).\n", getpid());



    }


    return 0;
}
