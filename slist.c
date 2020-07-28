#include "slist.h"

LInt removeID(LInt* head, int id){
  LInt tmp = *head, prev, res;
  LInt next = (*head)->prox;
  while(tmp != NULL && tmp->id == id){
    res = *head;
    *head = next;
    return res;
  }
  while(tmp != NULL && tmp->id != id){
    prev = tmp;
    tmp = tmp->prox;
  }
  res = tmp;
  if(tmp == NULL)
    return NULL;
  prev->prox =  tmp->prox;
  return res;
}


LInt newNode(char* comando, int id, int ordem, int termi){
  LInt aux = malloc(sizeof(struct slist));
  if(aux != NULL){
    aux->comando = strdup(comando);
    aux->id = id;
    aux->ordem = ordem;
    aux->terminacao = termi;
    aux->prox = NULL;
  }
  return aux;
}

void adiciona (LInt* head, char* comando, int id, int ordem, int termi){
  LInt new = newNode(comando, id, ordem, termi);
  LInt tmp = *head;
  if(*head == NULL){
    *head = new;
  }
  else{
    while(tmp->prox != NULL){
        tmp = tmp->prox;
    }
    tmp->prox = new;
  }
}


LInt reverseL (LInt l){
    if (l == NULL || l -> prox == NULL) return l;
    LInt act = l->prox;
    LInt ant = l;
    ant ->prox = NULL;
    LInt prx = l->prox;
    while (act ->prox != NULL) {
        prx = act->prox;
        act ->prox = ant;
        ant = act;
        act = prx;
    }
    act ->prox = ant;
    return act;
}

int elementoNaOrdem(int ordem,LInt l){
  LInt tmp = l;
  if(l == NULL) return -1;
  while(tmp != NULL && tmp->ordem != ordem)
    tmp = tmp->prox;
  if(tmp == NULL) return -1;
  return tmp->id;
}
