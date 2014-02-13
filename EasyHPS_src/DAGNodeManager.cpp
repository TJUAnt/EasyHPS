#include "DAGNodeManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pthread.h>
#include <string.h>

static NodeList* dag_node_finish_stack;

static NodeList* dag_node_computable_stack;

static pthread_mutex_t finish_stack_lock;

static pthread_mutex_t computable_stack_lock;

static int finish_node_num;

static int* register_table;

int get_finish_node_num()
{
    return finish_node_num;
}
void init_register_table()
{
    int node_sum;
    MPI_Comm_size(MPI_COMM_WORLD,&node_sum);
    register_table = (int*)malloc(node_sum*sizeof(int));
    memset(register_table,0,sizeof(register_table));
}

int *get_register_table()
{
    return register_table;
}
void init_dag_node_finish_stack()
{
    pthread_mutex_init(&finish_stack_lock,NULL);
    dag_node_finish_stack = (NodeList *)malloc(sizeof(NodeList));
    dag_node_finish_stack->next = NULL;
    //dag_node_finish_stack->pre = NULL;
    dag_node_finish_stack->node_id = -1;
    finish_node_num = 0;
}

void init_dag_node_computable_stack()
{
    pthread_mutex_init(&computable_stack_lock,NULL);
    dag_node_computable_stack = (NodeList *)malloc(sizeof(NodeList));
    dag_node_computable_stack->next = NULL;
    //dag_node_computable_stack->pre = NULL;
    dag_node_computable_stack->node_id = -1;
}

void add_dag_node_computable_stack(int dag_node_id)
{
    //    printf("add_dag_node_computable_stack %d\n",dag_node_id);
    pthread_mutex_lock(&computable_stack_lock);  
    NodeList *node = (NodeList*)malloc(sizeof(NodeList));
    node->next = dag_node_computable_stack->next;
    //node->pre = dag_node_computable_stack;
    node->node_id = dag_node_id;
    dag_node_computable_stack->next = node;
    pthread_mutex_unlock(&computable_stack_lock);
    return;
}

void add_dag_node_finish_stack(int dag_node_id)
{
    pthread_mutex_lock (&(finish_stack_lock));
    //pthread_mutex_lock()
    NodeList *node = (NodeList*)malloc(sizeof(NodeList));
    node->next = dag_node_finish_stack->next;
    //node->pre = dag_node_finish_stack;
    node->node_id = dag_node_id;
    dag_node_finish_stack->next = node;
    finish_node_num++;
    pthread_mutex_unlock(&(finish_stack_lock));
    return;
}
int pop_dag_node_finish_stack()
{
    pthread_mutex_lock(&(finish_stack_lock));
    NodeList *node_ptr = dag_node_finish_stack->next;
    if(node_ptr == NULL)
    {
	pthread_mutex_unlock(&(finish_stack_lock));
	return -1;
    }
    int ret = dag_node_finish_stack->next->node_id;
    dag_node_finish_stack->next = node_ptr->next;
    free(node_ptr);
    pthread_mutex_unlock(&(finish_stack_lock));
    node_ptr = NULL;
    return ret;
}

int pop_dag_node_computable_stack()
{
    pthread_mutex_lock(&computable_stack_lock);
    NodeList *node_ptr = dag_node_computable_stack->next;
    if(node_ptr == NULL)
    {
	pthread_mutex_unlock(&computable_stack_lock);
	return -1;
    }
    int ret = dag_node_computable_stack->next->node_id;
    dag_node_computable_stack->next = node_ptr->next;
    free(node_ptr);
    node_ptr = NULL;
    pthread_mutex_unlock(&computable_stack_lock);
    return ret;
}

void destroy_dag_node_finish_stack()
{
    while(pop_dag_node_finish_stack()!=-1){}
    free(dag_node_finish_stack);
    pthread_mutex_destroy(&finish_stack_lock);
    return;
}

void destroy_dag_node_computable_stack()
{
    while(pop_dag_node_computable_stack()!=-1){}
    free(dag_node_computable_stack);
    return;
}

void update_computable_node(int dag_node_id)
{
    int i,tmp_node_id;
    DAGPattern dag_pattern = get_dag_pattern();
    if(dag_node_id == -1)
    {
	int size = dag_pattern.rect_size.row * dag_pattern.rect_size.col;
	for(i = 0; i < size; i++)
	{
	    if(dag_pattern.dag_pattern_element[i].pre_cnt == 0)
	    {
		add_dag_node_computable_stack(i);
	    }
	}
	return;
    }

    int posfix = dag_pattern.dag_pattern_element[dag_node_id].pos_cnt;
    for(i = 0; i < posfix; i++)
    {
	tmp_node_id = dag_pattern.dag_pattern_element[dag_node_id].posfix_id[i];
	dag_pattern.dag_pattern_element[tmp_node_id].pre_cnt--;
	if(dag_pattern.dag_pattern_element[tmp_node_id].pre_cnt == 0)
	{
	    add_dag_node_computable_stack(tmp_node_id);
	}
    }
    return;
}

/**
 * collect finish node to find more computable node;
 */
void update_dag_pattern()
{
    int node_id;
    while(1)
    {
	node_id = pop_dag_node_finish_stack();
	if(node_id == -1)
	    return;
	update_computable_node(node_id);
    }
    return;
}

int is_computable_finished()
{
    DAGPattern dag_pattern = get_dag_pattern();

    if(finish_node_num == dag_pattern.rect_size.row * dag_pattern.rect_size.col)
	return 1;
    else
	return 0;
}
