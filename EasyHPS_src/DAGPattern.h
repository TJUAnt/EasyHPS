#ifndef _DAG_PATTERN_H_
#define _DAG_PATTERN_H_


typedef struct Size
{
    int row; // The number of rows of DAG
    int col; // The number of columns of DAG
}SizeT;

typedef struct Pos
{
    int x,y; // The x-axis,y-axis position of the node,node of DAG
}PosT;

typedef struct Block
{
    int node_id;
    PosT block_pos;
    SizeT block_size;
}BlockT;

typedef void (*Process_t)(int thread_id,SizeT dag_size, BlockT *block);

typedef struct tagDAGElement
{
    
    int pre_cnt; // prefix degree for a DAG node;
    int pos_cnt; // postfix degree for a DAG node;
    int data_pre_cnt;
    int *posfix_id; // link list for postfix node
    int *data_prefix_id;
    Process_t process;
}DAGElement;

enum DAG_pattern_type
{
    Left_Up_DAG,
    Left_Down_DAG,
};
typedef struct DAG
{
    DAGElement* dag_pattern_element;
    int* dag_pattern_finish;
    SizeT dag_size;
    SizeT block_size;
    SizeT rect_size;
    PosT dag_pos;
    enum DAG_pattern_type dag_pattern_type;
}DAGPattern;

//void init_dag_pattern(PosT dag_pos,SizeT dag_size, SizeT block_size, Process_t process);

void destroy_dag_pattern();

DAGPattern get_dag_pattern();

void SetDAGPattern(DAGElement* dagelementptr,SizeT dag_size,SizeT block_size,SizeT rect_size,PosT dag_pos, enum DAG_pattern_type dag_pattern_type,int *dag_pattern_finish);

BlockT* data_mapping_by_node_id(int dag_node_id);

Process_t get_process_by_node_id(int dag_node_id);

void send_datablock(int des_id,BlockT* data_block);

void send_datablockpos(int des_id,BlockT* data_block);

void recv_datablock(int src_id);

void write_datablock(int node_id);

void read_datablock(int node_id);

BlockT recv_datablockpos(int src_id);

void Init_DAG_lock();

#endif //_DAG_PATTERN_H_

