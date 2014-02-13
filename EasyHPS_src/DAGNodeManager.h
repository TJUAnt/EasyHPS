#ifndef _DAG_NODE_MANAGER_H_
#define _DAG_NODE_MANAGER_H_

#include "DAGPattern.h"
typedef struct
{
    int dag_node_id;
    BlockT dag_node_data_block;
}DAGNode;

typedef struct tagNodeList
{
    struct tagNodeList *next;
    //struct tagNodeList *pre;
    int node_id;
}NodeList;

void init_dag_node_finish_stack();

void init_dag_node_computable_stack();

void add_dag_node_finish_stack(int dag_node_id);

void add_dag_node_computable_stack(int dag_node_id);

void destroy_dag_node_finish_stack();

void destroy_dag_node_computable_stack();

int pop_dag_node_finish_stack();

int pop_dag_node_computable_stack();

void update_dag_pattern();

void update_computable_node(int dag_node_id);

int is_computable_finished();

int get_finish_node_num();

#endif
