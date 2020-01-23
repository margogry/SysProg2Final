#include <stdio.h>
#define _USE_BSD 1
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>


#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

#define MAX_DATA 1000

char *serverIP = "127.0.0.1";
int serverPort = 8088;
int sock;
//FILE *logFile;

void error(char *err) { // Вывод ошибок в терминал
    printf("%s\n", err);
    perror(err);
    exit(1);
}


int main(int argc,char *argv[])
{
    char *help = "\033[1mNAME\033[0m\n\t - UDP client, that send coordinates to server, that calculate square\n"
                 "\033[1mSYNOPSIS\033[0m\n\t[OPTIONS]\n"
                 "\033[1mDESCRIPTION\033[0m\n"
                 "\t-a=IP\n\t\tset server listening IP\n"
                 "\t-p=PORT\n\t\tset server listening PORT\n"
                 "\t-v\n\t\tcheck program version\n"
                 "\t-h\n\t\tprint help\n";

    int rez;

    if (getenv("L2ADDR") != NULL) {
        serverIP = getenv("L2ADDR");
    }
    if (getenv("L2PORT") != NULL) {
        serverPort = atoi(getenv("L2PORT"));
    }

    while ((rez = getopt(argc, argv, "a:p:vh")) != -1) {
        switch (rez) {
            case 'a':
                if (strncmp(optarg, "", 1) == 0) {
                    printf("%s", help);
                    return 0;
                }
                serverIP = optarg;
                break;
            case 'p':
                if (strncmp(optarg, "", 1) == 0) {
                    printf("%s", help);
                    return 0;
                }
                serverPort = atoi(optarg);
                break;
            case 'v':
                printf("version 34.0\n");
                return 0;
            default:
                printf("%s", help);
                return 0;
        }
    }

    socklen_t addrLength;

    sock = socket(AF_INET, SOCK_DGRAM,0);
    if(sock < 0)
        error("can't create socket\n");

    struct sockaddr_in serverAddress; 					 //an internet endpoint address
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;   				         //family name
    serverAddress.sin_port = htons(serverPort);//port number
    serverAddress.sin_addr.s_addr = inet_addr(serverIP);
    inet_pton(AF_INET, serverIP, &(serverAddress.sin_addr));                         //to convert host name into 32-bit IP address

    addrLength = sizeof(struct sockaddr);
    char *inputText = NULL;
    inputText = (char*)malloc(sizeof(char));
    int numberOfSymbolsInText = 0;

    printf("Waiting for data\n");

    char currentSymbol = fgetc(stdin);

    while (currentSymbol != '\n')//чтение
    {
        inputText = (char *) realloc(inputText, (numberOfSymbolsInText + 1) * sizeof(char));
        inputText[numberOfSymbolsInText] = currentSymbol;
        currentSymbol = fgetc(stdin);
        numberOfSymbolsInText++;
    }
    inputText = (char *) realloc(inputText, (numberOfSymbolsInText + 1) * sizeof(char));
    inputText[numberOfSymbolsInText] = currentSymbol;
    numberOfSymbolsInText++;
// request to send datagram
// no need to specify server address in sendto
// connect stores the peers IP and port
    sendto(sock, inputText, numberOfSymbolsInText, 0, (const struct sockaddr *) &serverAddress, addrLength);
    char answer[MAX_DATA];

    recvfrom(sock, answer, MAX_DATA, 0, // Получаем ответ на наш вопрос от сервера
             (struct sockaddr *) &serverAddress, &addrLength);

    printf("Server answer: %s\n", answer); // Вывод ответа

    inputText = NULL;
    free(inputText);
    close(sock);
    exit(0); //*/
}