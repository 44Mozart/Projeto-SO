#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct slist{
  char* comando;
  int id;
  int ordem;
  int terminacao;
  struct slist* prox;
}*LInt;


LInt removeID (LInt* head,  int id);
LInt newNode(char* comando, int id, int ordem, int termi);
void adiciona (LInt* head, char* comando, int id, int ordem, int termi);
LInt reverseL (LInt head);
int elementoNaOrdem(int ordem,LInt l);
