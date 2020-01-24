#include <stdio.h>
#define _USE_BSD 1
#include <string.h>
#include <math.h>
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
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/wait.h>

#define MAX_SEND_BUF 1000
#define MAX_RECV_BUF 1000
#define MAX_DATA 1000
#define  MAXLINE 1000
int sock, count, good, bad;
char *serverIP = "127.0.0.1";
int serverPort = 8088;
int delay = 0;
char *logPath = "/tmp/lab2.log";
clock_t begin;

void check(int retSig) {
    if (retSig < 0) {
        fprintf(stderr, "ERROR: failed to set proc mask: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

char *currentTimestamp() {
    time_t timer;
    char *buffer = (char *) malloc(sizeof(char) * 26);
    struct tm *tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%d.%m.%Y %H:%M:%S", tm_info);
    return buffer;
}
FILE *logFile = NULL;

void error(char *err) {
    if (logFile == NULL) {
        printf("%s\t%s\n", currentTimestamp(), err);
        fflush(stdin);
    } else {
        fprintf(logFile, "%s\t%s\n", currentTimestamp(), err);
        fflush(logFile);
        fclose(logFile);
    }
    perror(err);
    exit(1);
}

// Converts a floating-point/double number to a string.

struct Triangle{
    int errorCode;
    double coordinates[6];
    double square;
};

struct Triangle proceedLineToCoordinates(char * line){
    sleep(delay);
    struct Triangle currentTriangle;
    currentTriangle.errorCode = 0;
    currentTriangle.square = 0.0;
    int l=0;
    for (int j = 0; j < 6; j++){
        currentTriangle.coordinates[j] = 0;
    }
    int numberOfCoordinates = 0;
    printf("%s\n", line);
    int i = 0;
    double r;
    char* istr = NULL;
    istr = strtok(line, "\n\x20\x0B\x0C\t\r");
    while (istr != NULL) {
        l=0;
        for (i = 0; i < (int) strlen(istr); i++)
            if (!(((istr[i] >= '0') && (istr[i] <= '9'))||(istr[i] == '.')||(istr[i] == '+')||(istr[i] == '-'))){
                l++;
            }
        if(l>1){
            currentTriangle.errorCode = 4;
            return currentTriangle;
        }
        if (!(((istr[0] >= '0') && (istr[0] <= '9'))||(istr[0] == '+')||(istr[0] == '-')||(istr[0] == 'e')||(istr[0] == 'E'))){
            currentTriangle.errorCode = 4;
            return currentTriangle;
        }
        for (i = 1; i < (int) strlen(istr); i++){
            if (!(((istr[i] >= '0') && (istr[i] <= '9'))||(istr[i] == '.')||(istr[i] == '+')||(istr[i] == '-')||(istr[i] == 'e')||(istr[i] == 'E'))){
                currentTriangle.errorCode = 4;
                return currentTriangle;
            }
            if ((istr[i]=='+')||(istr[i]=='-')){
                if(!((istr[i-1]=='e')||(istr[i-1]=='E'))){
                    currentTriangle.errorCode = 4;
                    return currentTriangle;
                }
                if(i==(int)strlen(istr)){
                    currentTriangle.errorCode = 4;
                    return currentTriangle;
                }
            }
        }
        if (numberOfCoordinates>5){
            currentTriangle.errorCode = 1;
            return currentTriangle;
        }
        if(l==1){
            for (i = 0; i< (int) strlen(istr); i++){
                if(istr[i]=='e'){
                    sscanf(istr, "%le" , &currentTriangle.coordinates[numberOfCoordinates]);
                    break;
                }
                if(istr[i]=='E'){
                    sscanf(istr, "%lE" , &currentTriangle.coordinates[numberOfCoordinates]);
                    break;
                }
            }
        }else{
            sscanf(istr, "%lf" , &currentTriangle.coordinates[numberOfCoordinates]);
        }
        numberOfCoordinates++;
        istr = strtok(NULL, "\n\x20\x0B\x0C\t\r");
    }

    if (numberOfCoordinates != 6) {
        currentTriangle.errorCode = 1;
        bad++;
        return currentTriangle;
    }
    return currentTriangle;
}

struct Triangle getSquare(struct Triangle triangle){
    //printf("im here get square");
    if (triangle.errorCode == 0) {
        //printf("im here ger s in if");
        double x1 = triangle.coordinates[0];
        double y1 = triangle.coordinates[1];
        double x2 = triangle.coordinates[2];
        double y2 = triangle.coordinates[3];
        double x3 = triangle.coordinates[4];
        double y3 = triangle.coordinates[5];

        //printf("%lf %lf %lf %lf %lf %lf\n", x1, y1, x2, y2, x3, y3);
        double S = 0.5 * abs((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1));

        triangle.square = S;

        if (triangle.square == 0.0){
            triangle.errorCode = 3;
            printf("ERROR 3 not a triangle\n");
            fprintf(logFile, "%s\tЗавершение обслуживания запроса\n", currentTimestamp());
            bad++;
            fflush(logFile);
            return triangle;
        }
    }
    fprintf(logFile, "%s\tЗавершение обслуживания запроса\n", currentTimestamp());
    count++;
    good++;
    fflush(logFile);
    return triangle;
}


void quit() {
    printf("\n%s\tЗавершение работы сервера\n", currentTimestamp());
    fprintf(logFile, "%s\tЗавершение работы сервера\n", currentTimestamp());
    fflush(logFile);
    close(sock);
    exit(0);
}


char *itoa(int number) {
  char *destination;
  destination = (char *) malloc(32);
  int count = 0;
  do {
    int digit = number % 10;
    destination[count++] = (digit > 9) ? digit - 10 +'A' : digit + '0';
  } while ((number /= 10) != 0);
  destination[count] = '\0';
  int i;
  for (i = 0; i < count / 2; ++i) {
    char symbol = destination[i];
    destination[i] = destination[count - i - 1];
    destination[count - i - 1] = symbol;
  }
  return destination;
}

void quitWithLog() {
    clock_t end = clock();
    char tmp[512]={0};
    double time_spent = (double) (end - begin) / CLOCKS_PER_SEC; 
    //sprintf(tmp,"Сервер работал: %f с\nОбслуженные запросы: %d\nТекущее время: %s\nКоличество обработанных запросов: %d\nКоличество ошибочных запросов: %d\n", time_spent, count, currentTimestamp(), good, bad);
    strcat(tmp,"Сервер работал: \0"); strcat(tmp, itoa(time_spent)); strcat(tmp,"\nОбслуженные запросы: \0"); 
    strcat(tmp, itoa(count)); strcat(tmp,"\nТекущее время: \0"); strcat(tmp, currentTimestamp()); 
    strcat(tmp,"\nКоличество обработанных запросов: \0"); strcat(tmp, itoa(good)); 
    strcat(tmp,"\nКоличество ошибочных запросов: \0"); strcat(tmp, itoa(bad));
 
    write(fileno(logFile),tmp,strlen(tmp));
    write(STDOUT_FILENO,tmp,strlen(tmp));
}


int errexit(const char* format,...)
{
    va_list args;

    va_start(args,format);
    vfprintf(stderr,format,args);
    va_end(args);
    exit(1);
}

/*
 main - connectionless multiprocess server
 */
int main(int argc,char *argv[]){
    char *help = "\033[1mNAME\033[0m\n\t - UDP server, that calculate square\n"
                 "\033[1mSYNOPSIS\033[0m\n\t[OPTIONS]\n"
                 "\033[1mDESCRIPTION\033[0m\n"
                 "\t-w=N\n\t\tset delay N for client \n"
                 "\t-d\n\t\tdaemon\n"
                 "\t-l=path/to/log\n\t\tPath to log-file\n"
                 "\t-a=IP\n\t\tset server listening IP\n"
                 "\t-p=PORT\n\t\tset server listening PORT\n"
                 "\t-v\n\t\tcheck program version\n"
                 "\t-h\n\t\tprint help\n";
    int rez;
    int daemonFlag = 0;
    begin = clock();
    struct sigaction sa = {0};
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_sigaction = quit;
    sa.sa_flags |= (SA_SIGINFO | SA_RESTART);
    check(sigaction(SIGINT, &sa, NULL));
    check(sigaction(SIGQUIT, &sa, NULL));
    check(sigaction(SIGTERM, &sa, NULL));
    struct sigaction sa1 = {0};
    sigemptyset(&sa1.sa_mask);
    sigaddset(&sa1.sa_mask, SIGUSR1);
    sa1.sa_sigaction = quitWithLog;
    sa1.sa_flags |= (SA_SIGINFO | SA_RESTART);
    check(sigaction(SIGUSR1, &sa1, NULL));
    if (getenv("L2ADDR") != NULL) {
        serverIP = getenv("L2ADDR");
    }
    if (getenv("L2PORT") != NULL) {
        serverPort = atoi(getenv("L2PORT"));
    }
    if (getenv("L2WAIT") != NULL) {
        delay = atoi(getenv("L2WAIT"));
    }
    while ((rez = getopt(argc, argv, "w:dl:a:p:vh")) != -1) {
        switch (rez) {
            case 'w':
                delay = atoi(optarg);
                break;
            case 'd':
                daemonFlag = 1;
                break;
            case 'l':
                if (strncmp(optarg, "", 1) == 0) {
                    printf("%s", help);
                    return 0;
                }
                logPath = strdup(optarg);
                break;
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
                printf("\033version 36\n");
                return 0;
            default:
                printf("%s", help);
                return 0;
        }
    }

    if (daemonFlag == 1) {
        pid_t pid, sid;
        pid = fork();
        if (pid < 0) {
            error("ERROR 99 Fork failed ");
        }
        if (pid > 0) {
            exit(0);
        }

        sid = setsid();
        if (sid < 0) {
            error("ERROR 98 SID failed ");
        }
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler = SIG_IGN;
        sa.sa_flags = 0;
        sigaction(SIGCHLD, &sa, NULL);
        sigaction(SIGHUP, &sa, NULL);
        if ((pid = fork()) < 0) {
            error("ERROR 99 Fork failed ");
        }
        if (pid > 0) {
            exit(0);
        }
        umask(0);
        chdir("/");
        int fd;
        for (fd = sysconf(_SC_OPEN_MAX); fd > 0; --fd) {
            close(fd);
        }
        stdin = fopen("/dev/null", "r");
        stdout = fopen("/dev/null", "w+");
        stderr = fopen("/dev/null", "w+");
        if ((logFile = fopen(logPath, "rb")) == NULL) {
            error("ERROR 24: Open file");
        }

    }else{
        if ((logFile = fopen(logPath, "a+b")) == NULL) {
            error("ERROR 24: Open file");
        }
    }
    printf("Connectsock\n");
    struct sockaddr_in server;                                                //an internet endpoint address


    fprintf(logFile, "\n%s\tНачало работы сервера\n", currentTimestamp());
    fflush(logFile);
    sock = socket(AF_INET, SOCK_DGRAM, 0);                                    //allocate a socket

    if(sock<0)
    {
        printf("Socket can't be created\n");
        exit(0);
    }
    fprintf(logFile, "%s\tSocket created\n", currentTimestamp());
    fflush(logFile);
/* to set the socket options- to reuse the given port multiple times */
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed");
        exit(0);
    }
    fcntl(sock, F_SETFL, O_NONBLOCK);
    memset(&server, 0, sizeof(server));

    server.sin_addr.s_addr= htons(INADDR_ANY);                                 //INADDR_ANY to match any IP address
    server.sin_family = AF_INET;                                                //family name
    server.sin_port = htons(serverPort);                                              //port number
/* bind the socket to known port */
    if(bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        printf("Error in binding\n");
        exit(0);
    }
    fprintf(logFile, "%s\tBinding done\n", currentTimestamp());
    fflush(logFile);
    printf("Listening to clients\n");

    while (1) {
        socklen_t addrLength = sizeof(struct sockaddr);
        char data[MAX_DATA];
        memset(data, 0, MAX_DATA);
        int messLength = recvfrom(sock, (char *)data, sizeof(data), 0,  (struct sockaddr *) &server, &addrLength);
        if(messLength>0){
            struct Triangle result;
            fprintf(logFile, "%s\tПоступление запроса\n", currentTimestamp());
            fflush(logFile);
            switch (fork()) {
                case 0: {/* child */
                    result = getSquare(proceedLineToCoordinates(data));

                    char *message;

                    if (result.errorCode == 0) {
                        message = (char *) malloc(20 * sizeof(char));
                        sprintf(message,"%lf", result.square);
                        sendto(sock, message, MAXLINE, 0, (struct sockaddr *) &server, addrLength);
                        exit(0);
                    } else {
                        message = (char *) malloc(9 * sizeof(char));
                        strcpy(message, "ERROR ");
                        message[6] = (char) (result.errorCode + (int) '0');
                        sendto(sock, message, MAXLINE, 0, (struct sockaddr *) &server, addrLength);
                        exit(1);
                    }
                }
                default: /* parent */
                    break;
                case -1:
                    printf("error in forking\n");
            }
        }
        int status = -1;
        if (waitpid(-1, &status, WNOHANG) > 0) {
            if (status==0){
                count++;
                good++;
            }
            else {
                bad++;
                count++;
            }
        }
    }
    close(sock);
    return 0;
}
