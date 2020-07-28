#include "argus.h"
#include "slist.h"

#define BUFFER_SIZE 100

int max_inat = 0;
int max_exec = 0;
int log_fd;
int log_idx;
int fifo_resposta;
int fifo_fd;
LInt historico;
LInt execucao;
int tam_historico = 0;
int tam_total = 0;
int tam_execucao = 0;
int* pids;
int nFilhos = 0;


void sigchld_handler(int signum){
  int status;
  int pid = wait(&status);
  LInt aux = removeID(&execucao,pid);
  tam_execucao--;
  adiciona(&historico,aux->comando,aux->id, aux->ordem, WEXITSTATUS(status));
  tam_historico++;
  int p = lseek(log_fd,0,SEEK_END);
  lseek(log_idx,0,SEEK_END);
  write(log_idx,&p,sizeof(int));
  free(aux->comando);
  free(aux);
  printf("Morreu %d, %d\n",pid, WEXITSTATUS(status));
}

//Execucao
void sigalrm1_handler(int signum){
  printf("Morreu por tempo de execucao\n");
  exit(2);
}

//Inatividade
void sigalrm2_handler(int signum){
  for(int i = 0; i <  nFilhos;  i++){
    kill(pids[i],SIGKILL);
  }
  printf("Morreu por tempo inactividade\n");
  exit(3);
}

void sigterm_handler(int signum){
  for(int i = 0; i <  nFilhos;  i++){
    kill(pids[i],SIGKILL);
  }
  printf("Terminado.\n");
  exit(4);
}

int isPipe(char* buf){
  int i = 0,flag = 0;
  while(buf[i]){
    if(buf[i] == '|')
      flag++;
    i++;
  }
  return flag;
}

void mysystem(char* command, int log_fd){
	pid_t pid;
	char* exec_args[20];
	char* string;
	int i;
	string = strtok(command, " ");
	for (i = 0; string != NULL; i++) {
		exec_args[i] = string;
		string = strtok(NULL, " ");
	}
	exec_args[i] = NULL;
  if(signal(SIGCHLD,SIG_DFL) == SIG_ERR){
    perror("Erro ao criar sinal");
    exit(1);
  }

  if(signal(SIGALRM,sigalrm1_handler) == SIG_ERR){
    perror("Erro ao criar sinal de alarme");
    exit(1);
  }

  if(signal(SIGTERM,sigterm_handler) == SIG_ERR){
    perror("Erro a matar");
    exit(1);
  }
  pids = malloc(sizeof(int));
  alarm(max_exec);
	if ((pid = fork()) == 0) {
    dup2(log_fd,1);
		execvp(exec_args[0], exec_args);
	}
	else if (pid > 0) {
    pids[nFilhos++] = pid;
		wait(NULL);
    alarm(0);
    _exit(0);
	}
}

int exec_command(char* command){
    char* exec_args[20];
    char* string;
    int exec_ret = 0;
    int i = 0;
    string = strtok(command," ");
    while (string != NULL) {
        exec_args[i] = string;
        string = strtok(NULL," ");
        i++;
    }
    exec_args[i] = NULL;
    exec_ret = execvp(exec_args[0], exec_args);
    return exec_ret;
}

void dividePipes(int nPipes,char* buf,char** pipes){
  char* string;
  int r = 0;
  string =  strtok(buf,"|");
  pipes[r++] = string;
  while(string){
    if((string = strtok(NULL,"|")) == NULL)
      break;
    pipes[r++] = string;
  }
}

int executa(int nPipes, char** p,int log_fd){
    int n = nPipes+1;
    int status;                                // número de comandos == número de filhos
    int a[n-1][2];                                     // -> matriz com os fd's dos pipes                                    // -> array que guarda o return dos filhos
    // criar os pipes conforme o número de comandos
    pids = malloc(sizeof(int));
    int pid;

    if(signal(SIGCHLD,SIG_DFL) == SIG_ERR){
      perror("Erro ao criar sinal");
      exit(1);
    }

    if(signal(SIGALRM,sigalrm1_handler) == SIG_ERR){
      perror("Erro ao criar sinal de alarme");
      exit(1);
    }

    if(signal(SIGALRM,sigalrm2_handler) == SIG_ERR){
      perror("Erro ao criar sinal de alarme");
      exit(1);
    }

    if(signal(SIGTERM,sigterm_handler) == SIG_ERR){
      perror("Erro a matar");
      exit(1);
    }

    alarm(max_exec);
    if((pid = fork()) == 0){
      for (int i = 0; i < n-1; i++) {
          if (pipe(a[i]) == -1) {
              perror("Pipe não foi criado");
              return -1;
          }
      }
      // criar processos filhos para executar cada um dos comandos
      for (int i = 0; i < n; i++) {
        if (i == 0) {
          switch(pid = fork()) {
              case -1:
                  perror("Fork não foi efetuado");
                  return -1;
              case 0:
                  // codigo do filho 0
                  close(a[i][0]);
                  dup2(a[i][1],1);
                  close(a[i][1]);
                  exec_command(p[i]);
                  _exit(0);
              default:
                  close(a[i][1]);
                  alarm(max_inat);
                  pids[nFilhos++] = pid;
                  wait(NULL);
          }
        }
        else if (i == n-1) {
            switch(pid = fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho n-1
                    dup2(a[i-1][0],0);
                    close(a[i-1][0]);
                    dup2(log_fd,1);
                    exec_command(p[i]);
                    _exit(0);
                default:
                    close(a[i-1][0]);
                    pids[nFilhos++] = pid;
                    wait(NULL);
                    alarm(0);
                    exit(0);
            }
        }
        else {
            switch(pid = fork()) {
                case -1:
                    perror("Fork não foi efetuado");
                    return -1;
                case 0:
                    // codigo do filho i
                    dup2(a[i-1][0],0);
                    close(a[i-1][0]);
                    dup2(a[i][1],1);
                    close(a[i][1]);
                    exec_command(p[i]);
                    _exit(0);
                default:
                    close(a[i-1][0]);
                    close(a[i][1]);
                    alarm(alarm(0));
                    pids[nFilhos++] = pid;
                    wait(NULL);
            }
          }
        }
      }
      else{
        pids[nFilhos++] = pid;
        wait(&status);
        alarm(0);
        exit(WEXITSTATUS(status));
      }
    for (int i = 0; i < n; i++)
        wait(NULL);
  return 0;
}


int defineComando(char* com){
  int e = -1;
  if(strcmp(com,"tempo-inactividade") == 0 || strcmp(com,"-i") == 0)
    e = 1;
  else if(strcmp(com,"tempo-execucao") == 0 || strcmp(com,"-m") == 0)
    e = 2;
  else if(strcmp(com,"executar") == 0)
    e = 3;
  else if(strcmp(com,"-e") == 0)
    e = 4;
  else if(strcmp(com,"listar") == 0 || strcmp(com,"-l") == 0)
    e = 5;
  else if(strcmp(com,"terminar") == 0 || strcmp(com,"-t") == 0)
    e = 6;
  else if(strcmp(com,"historico") == 0 || strcmp(com,"-r") == 0)
    e = 7;
  else if(strcmp(com,"ajuda") == 0 || strcmp(com,"-h") == 0)
    e = 8;
  else if(strcmp(com,"output") == 0 || strcmp(com,"-o") == 0)
    e = 9;
  return e;
}

int quantosDigitos(int ordem){
  int quant = 0;
  while(ordem > 0){
    quant++;
    ordem = ordem/10;
  }
  return quant;
}

const char* linkedToStringHistorico(){
  char* buf = malloc(sizeof(char));
  if(historico == NULL) {
    snprintf(buf,2,"\n");
  }
  else{
    LInt l = historico;
    while(l != NULL){
      if(l->terminacao == 0){
        snprintf(buf+strlen(buf),strlen(l->comando)+ 17 +quantosDigitos(l->ordem),"#%d, concluida: %s\n", l->ordem, l->comando);
      }
      else if(l->terminacao == 2){
        snprintf(buf+strlen(buf),strlen(l->comando)+ 20 +quantosDigitos(l->ordem),"#%d, max execucao: %s\n", l->ordem, l->comando);
      }
      else if(l->terminacao == 3){
        snprintf(buf+strlen(buf),strlen(l->comando)+ 23 +quantosDigitos(l->ordem),"#%d, max inatividade: %s\n", l->ordem, l->comando);
      }
      else {// l->terminacao  = 4;
        snprintf(buf+strlen(buf),strlen(l->comando)+ 17 +quantosDigitos(l->ordem),"#%d, terminada: %s\n", l->ordem, l->comando);
      }
      l = l->prox;
    }
  }
  return buf;
}

const char* linkedToString(){
  char* buf = malloc(sizeof(char));
  if(execucao == NULL) {
    snprintf(buf,2,"\n");
  }
  else{
    LInt l = execucao;
    while(l != NULL){
      snprintf(buf+strlen(buf),strlen(l->comando)+ 5 +quantosDigitos(l->ordem),"#%d: %s\n", l->ordem, l->comando);
      l = l->prox;
    }
  }
  return buf;
}

const char* ajudaMenu(){
  char* buf = malloc(sizeof(char));
  snprintf(buf,2,"\n");
  snprintf(buf+strlen(buf),25,"tempo-inactividade segs\n");
  snprintf(buf+strlen(buf),21,"tempo-execucao segs\n");
  snprintf(buf+strlen(buf),29,"executar p1 | p2 | ... | pn\n");
  snprintf(buf+strlen(buf),8,"listar\n");
  snprintf(buf+strlen(buf),12,"terminar n\n");
  snprintf(buf+strlen(buf),11,"historico\n");
  snprintf(buf+strlen(buf),7,"ajuda\n");
  snprintf(buf+strlen(buf),10,"output n\n");
  snprintf(buf+strlen(buf),2,"\n");
  return buf;
}

int main(int argc, char* argv[]){
  fifo_resposta = open("fifo_resposta",O_WRONLY);
  if(fifo_resposta < 0){
    perror("Erro a abrir fifo resposta");
    exit(1);
  }
  log_fd = open("log.txt",O_CREAT | O_TRUNC | O_RDWR,0666);
  if(log_fd < 0){
    perror("Erro abrir log.");
    exit(1);
  }
  log_idx = open("log.idx",O_CREAT | O_TRUNC | O_RDWR,0666);
  if(log_idx < 0){
    perror("Erro abrir log.idx");
    exit(1);
  }
  historico = NULL;
  execucao = NULL;
  while ((fifo_fd = open("fifo",O_RDONLY)) > 0) {
    char buf[BUFFER_SIZE];
    int bytes_read = read(fifo_fd, buf, BUFFER_SIZE);
    char* pipes[BUFFER_SIZE];
    if(bytes_read > 0){
      char* comando = strtok(buf," ");
      char* valor  = strtok(NULL,"\0");
      int escolha = defineComando(comando);
      int pid = -1;
      switch (escolha) {
        case 1:{
          max_inat = atoi(valor);
          write(fifo_resposta,"",1);
          break;
        }
        case 2:{
          max_exec = atoi(valor);
          write(fifo_resposta,"",1);
          break;
        }
        case 3:{
          if(valor[0] == 39 && valor[strlen(valor)-1] == 39){
            int i;
            for(i = 1; valor[i]; i++)
                valor[i-1] = valor[i];
            valor[i-2] = '\0';
            if((pid = fork()) == 0) {
              int nPipes = isPipe(valor);
              if(nPipes == 0)
                mysystem(valor,log_fd);
              else if(nPipes > 0){
                dividePipes(nPipes,valor,pipes);
                executa(nPipes,pipes,log_fd);
              }
            }
            else if(pid < 0){
              perror("Erro no fork");
              _exit(1);
            }
            else{
              if(signal(SIGCHLD,sigchld_handler) == SIG_ERR){
                perror("Erro no sinal do filhos");
                _exit(1);
              }
              adiciona(&execucao,valor,pid,tam_total+1,-1);
              tam_execucao++;
              tam_total++;
            }
            char* a = malloc(sizeof(char));
            if(tam_total/10 > 0){
              snprintf(a,17,"nova tarefa #%d\n",tam_total);
            }
            else {
              snprintf(a,16,"nova tarefa #%d\n",tam_total);
            }
            write(fifo_resposta,a,16);
            //free(a);
          }
          else{
            write(fifo_resposta,"",1);
          }
          break;
        }
        case 4:{
          if((pid = fork()) == 0) {
            int nPipes = isPipe(valor);
            if(nPipes == 0)
              mysystem(valor,log_fd);
            else if(nPipes > 0){
              dividePipes(nPipes,valor,pipes);
              executa(nPipes,pipes,log_fd);
            }
          }
          else if(pid < 0){
            perror("Erro no fork");
            _exit(1);
          }
          else{
            if(signal(SIGCHLD,sigchld_handler) == SIG_ERR){
              perror("Erro no sinal do filhos");
              _exit(1);
            }
            adiciona(&execucao,valor,pid,tam_total+1,-1);
            tam_execucao++;
            tam_total++;
          }
          char* a = malloc(sizeof(char));
          if(tam_total/10 > 0){
            snprintf(a,17,"nova tarefa #%d\n",tam_total);
          }
          else {
            snprintf(a,16,"nova tarefa #%d\n",tam_total);
          }
          write(fifo_resposta,a,16);
          //free(a);
          break;
        }
        case 5:{
          const char* buf = linkedToString();
          write(fifo_resposta,buf,strlen(buf));
          break;
        }
        case 6:{
          int ordem = atoi(valor);
          if(tam_total >= ordem){
            int pid = elementoNaOrdem(ordem,execucao);
            if(pid > 0)
              kill(pid,SIGTERM);
          }
          write(fifo_resposta,"",1);
          break;

        }
        case 7:{
          const char* buf = linkedToStringHistorico();
          write(fifo_resposta,buf,strlen(buf));
          break;
        }
        case 8:{
          const char* ajuda = ajudaMenu();
          write(fifo_resposta,ajuda,strlen(ajuda));
          break;
        }
        case 9:{
          int e = atoi(valor);
          if(tam_historico >= e && e == 1){
            lseek(log_idx,0,SEEK_SET);
            int p;
            read(log_idx,&p,sizeof(int));
            printf("%d\n", p);
            lseek(log_fd,0,SEEK_SET);
            char buf[p+1];
            read(log_fd,buf,sizeof(char)*p);
            buf[p] = '\0';
            write(fifo_resposta,buf,strlen(buf));
          }
          else if(tam_historico >= e && e > 1){
            lseek(log_idx,sizeof(int)*(e-2),SEEK_SET);
            int inicio;
            int fim;
            read(log_idx,&inicio,sizeof(int));
            read(log_idx,&fim,sizeof(int));
            printf("%d\n",fim-inicio );
            lseek(log_fd,inicio,SEEK_SET);
            char buf[fim-inicio+1];
            read(log_fd,buf,sizeof(char)*(fim-inicio));
            buf[fim-inicio] = '\0';
            write(fifo_resposta,buf,strlen(buf));
          }
          else{
            write(fifo_resposta,"",1);
          }
          lseek(log_fd,0,SEEK_END);
          break;
        }
        default:
          write(fifo_resposta,"Comando errado\n",16);
          break;
      }
    }
  }
  return 0;
}
