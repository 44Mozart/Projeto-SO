#include "argus.h"
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]){
  int fifo_resposta = open("fifo_resposta",O_RDONLY);
  int fifo_fd = open("fifo",O_WRONLY);
  if(fifo_resposta < 0){
    perror("Erro no fifo resposta");
    exit(1);
  }
  if(fifo_fd < 0){
    perror("Erro abrir fifo.");
    exit(1);
  }
  char buf[BUFFER_SIZE];
  int k = 0,i,j;
  for(i = 1; i < argc; i++){
    for(j = 0; argv[i][j]; j++, k++){
      buf[k] = argv[i][j];
    }
    buf[k] = ' ';
    k++;
  }
  buf[k-1] = '\0';
  if(argc == 1){
    int bytes_read;
    char aux[BUFFER_SIZE];
    printf("argus$ ");
    fflush(stdout);
    while((bytes_read = read(0,buf,BUFFER_SIZE)) > 0){
      buf[bytes_read-1] = '\0';
      write(fifo_fd,buf,bytes_read);
      int bytes = read(fifo_resposta,aux,BUFFER_SIZE);
      write(1,aux,bytes);
      printf("argus$ ");
      fflush(stdout);
    }
  }
  else{
    char aux[BUFFER_SIZE];
    write(fifo_fd,buf,sizeof(char)*k);
    int bytes = read(fifo_resposta,aux,BUFFER_SIZE);
    write(1,aux,bytes);
  }
  close(fifo_fd);
  close(fifo_resposta);

  return 0;
}
