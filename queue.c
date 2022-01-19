#include <stdio.h>
#include <stdlib.h>

typedef struct node{
    int a,b,c,d;
} node;
typedef struct item{
    node* n;
    struct item* next;
} item;

typedef struct queue{
    struct item* init;
    struct item* last;
} queue;

node* pop(queue* q){
    if(q == NULL) return NULL;
    if(q->init == NULL) return NULL;
   item* it = q->init;
   node* n = q->init->n;
   if(q->init == q->last){
       q->last = NULL;
       q->init = NULL;
   }
   else{
       q->init = it->next;
   }
   printf("item poped\nitem addr: %p\nnode addr: %p\n",it,n);
   free(it);
   return n;
}

void push(queue* q,node* n){
    if(q == NULL || n == NULL) return;
    item* it = (item*) malloc(sizeof(item));
    it->next = NULL;
    it->n = n;
    if(q->init == NULL){
        q->init = it;
        q->last = it;
    }
    else{
        
        q->last->next = it;
        q->last = it;
    }
    printf("item pushed\nitem add: %p\nnode addr: %p\n",it,n);
}

int main(){
    queue* q = (queue*)malloc(sizeof(queue));
    
    int i;
    for(i=0;i<10;i++){
        push(q,(node*)malloc(sizeof(node)));
    }
    while(q->init != NULL){
        node* n = pop(q);
        printf("addr of node: %p\n",n);
        free(n);
    }
    free(q);
    return 0;
}