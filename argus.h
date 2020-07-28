#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

void sigchld_handler(int signum);
void sigalrm1_handler(int signum);
void sigalrm2_handler(int signum);
void sigterm_handler(int signum);
int isPipe(char* buf);
void mysystem(char* command, int log_fd);
int exec_command(char* command);
void dividePipes(int nPipes,char* buf,char** pipes);
int executa(int nPipes, char** p,int log_fd);
int defineComando(char* com);
int quantosDigitos(int ordem);
const char* linkedToStringHistorico();
const char* linkedToString();
const char* ajudaMenu();
int main(int argc, char* argv[]);
