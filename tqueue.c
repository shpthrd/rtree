#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4
#define M 6
#define K 10000000 //limit

typedef struct node{
    long x1,y1,x2,y2;
    struct node* child[M+1]; 
} node;
typedef struct item{
    node* n;
    struct item* next;
} item;

typedef struct queue{
    struct item* init;
    struct item* last;
} queue;


long counting_node = 0;
long counting_free_node = 0;
long counting_pop = 0;
long counting_push = 0;
queue* q;
node s_node;
pthread_mutex_t mut_f = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut_l = PTHREAD_MUTEX_INITIALIZER;
clock_t start, end;
double cpu_time_used;


//=====================================================================================================
//NODE FUNCTIONS
//=====================================================================================================

node* createNode(long x1,long y1, long x2,long y2){
    node* n = (node*)malloc(sizeof(node));
    counting_node++;
    n->x1 = x1;
    n->y1 = y1;
    n->x2 = x2;
    n->y2 = y2;
    if(x2-x1 > 1){
        long i;
        for(i=0;i<M;i++){
            n->child[i] = createNode(x1 + i*(x2 - x1)/M,x1 + i*(x2 - x1)/M,x1 + (i+1)*(x2 - x1)/M,x1 + (i+1)*(x2 - x1)/M);
        }
    }
    return n;
}

void printTree(node* n){
	long i;
    //counting_node++;
	printf("(%ld,%ld)(%ld,%ld)\n",n->x1,n->y1,n->x2,n->y2);
	i=0;
	while(n->child[i] != NULL && i<=M){
			printTree(n->child[i]);
			i++;
	}
}

long Overlap(node* r1, node* r2){
    if(r1 == NULL || r2 == NULL) return 0;
	long overX =0, overY = 0;
	if((r1->x1 <= r2->x1 && r1->x2 >= r2->x1) || (r2->x1 <= r1->x1 && r2->x2 >= r1->x1)){
		overX = 1;
	}
	if((r1->y1 <= r2->y1 && r1->y2 >= r2->y1) || (r2->y1 <= r1->y1 && r2->y2 >= r1->y1)){
		overY = 1;
	}
	if(overY ==1 && overX == 1) return 1; else return 0;
    
}

void freeNode(node** node){
	if(node == NULL) return;
	long i = 0;
	for(i = 0; i < M+1; i++){
		if((*node)->child[i] != NULL)
			freeNode(&(*node)->child[i]);
	}
	free(*node);
	*node = NULL;
    counting_free_node++;
}


//=====================================================================================================
//QUEUE FUNCTIONS
//=====================================================================================================

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

 


//=====================================================================================================
//PTHREAD FUNCTIONS
//=====================================================================================================
void *search(void* param)
{
    while(1){
        pthread_mutex_lock(&mut_f);
        pthread_mutex_lock(&mut_l);
        if(q->init == NULL){
            if(counting_pop == counting_node){
                end = clock();
	            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	            printf("time elapsed: %f\n",cpu_time_used);
                pthread_mutex_unlock(&mut_f);
                pthread_mutex_unlock(&mut_l);
                break;
            }
            pthread_mutex_unlock(&mut_f);
            pthread_mutex_unlock(&mut_l);
        }
        else{
            if(q->init != q->last){
                pthread_mutex_unlock(&mut_l);
                item* it = q->init;
                node* n = q->init->n;
                q->init = it->next;
                free(it);
                counting_pop++;
                //printf("%ld: pop(%ld,%ld)(%ld,%ld) - counting: %ld\n",pthread_self(),n->x1,n->y1,n->x2,n->y2,counting_pop);
                pthread_mutex_unlock(&mut_f);
                if(Overlap(n,&s_node) == 1){
                    //printf("here if\n");
                    pthread_mutex_lock(&mut_f);
                    pthread_mutex_lock(&mut_l);
                    long i = 0;
                    while(n->child[i] != NULL && i<M){
                        item* it2 = (item*) malloc(sizeof(item));
                        it2->next = NULL;
                        it2->n = n->child[i];
                        if(q->init == NULL){
                            q->init = it2;
                            q->last = it2;
                        }
                        else{
                            q->last->next = it2;
                            q->last = it2;
                        }
                        counting_push++;
                        //printf("%ld: push(%ld,%ld)(%ld,%ld) - counting %ld\n",pthread_self(),it2->n->x1,it2->n->y1,it2->n->x2,it2->n->y2,counting_push);
                        i++;
                    }
                    
                    pthread_mutex_unlock(&mut_f);
                    pthread_mutex_unlock(&mut_l);
                }
            }
            else{
                item* it = q->init;
                node* n = q->init->n;
                q->init = NULL;
                q->last = NULL;
                free(it);
                pthread_mutex_unlock(&mut_f);
                pthread_mutex_unlock(&mut_l);
                counting_pop++;
                //printf("%ld: pop(%ld,%ld)(%ld,%ld) - counting: %ld\n",pthread_self(),n->x1,n->y1,n->x2,n->y2,counting_pop);
                if(Overlap(n,&s_node) == 1){
                    //printf("here else\n");
                    pthread_mutex_lock(&mut_f);
                    pthread_mutex_lock(&mut_l);
                    long i = 0;
                    while(n->child[i] != NULL && i<M){
                        item* it2 = (item*) malloc(sizeof(item));
                        it2->next = NULL;
                        it2->n = n->child[i];
                        if(q->init == NULL){
                            q->init = it2;
                            q->last = it2;
                        }
                        else{
                            q->last->next = it2;
                            q->last = it2;
                        }
                        counting_push++;
                        //printf("%ld: push(%ld,%ld)(%ld,%ld) - counting %ld\n",pthread_self(),it2->n->x1,it2->n->y1,it2->n->x2,it2->n->y2,counting_push);
                        i++;
                    }
                    
                    pthread_mutex_unlock(&mut_f);
                    pthread_mutex_unlock(&mut_l);
                }
            }
        }
    }
    return 0;
}

int main(){
    q = (queue*)malloc(sizeof(queue));
    start = clock();
    node* root = createNode(0,0,K,K);
    end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("time elapsed create: %f\n",cpu_time_used);
    s_node.x1=0;
    s_node.y1=0;
    s_node.x2=K;
    s_node.y2=K;
    printf("counting node: %ld\n",counting_node);
    /* //printTree(root);
    printf("counting node: %ld\n",counting_node);
    start = clock();
    push(q,root);

    pthread_t threads[NUM_THREADS];
    long i;
    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, search, NULL);
    }

    for (i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    } */
    free(q);
    freeNode(&root);
    printf("counting free node: %ld\n",counting_free_node);
    exit(EXIT_SUCCESS);
}