#include "DAGPatternMaster.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/**
 * init dag pattern
 * !!! set pattern data_size and block_size first !!!
 */
void Master_init_dag_pattern(PosT dag_pos, SizeT dag_size, SizeT block_size, Process_t process, enum DAG_pattern_type dag_pattern_type)
{
//    printf("Master_init_dag_pattern\n");
    Init_DAG_lock();
    int i,j,cnt,p,k;
    //int total_dag_node_num;
    DAGPattern dag_pattern = get_dag_pattern();
    dag_pattern.dag_pos = dag_pos;
    dag_pattern.dag_size = dag_size;
    dag_pattern.block_size = block_size;
    dag_pattern.dag_pattern_type = dag_pattern_type;
    SizeT rect_dag_pattern;
    rect_dag_pattern.row = dag_size.row/block_size.row;
    if(dag_size.row%block_size.row != 0)
        rect_dag_pattern.row += 1;
    rect_dag_pattern.col = dag_size.col/block_size.col;
    if(dag_size.col%block_size.col != 0)
        rect_dag_pattern.col += 1;
    dag_pattern.rect_size = rect_dag_pattern;
    dag_pattern.dag_pattern_element = (DAGElement *)malloc(rect_dag_pattern.row * rect_dag_pattern.col*sizeof(DAGElement));
    dag_pattern.dag_pattern_finish = (int*)malloc(rect_dag_pattern.row * rect_dag_pattern.col * sizeof(int));
    for(i = 0; i < rect_dag_pattern.row; i++){
	for(j = 0; j < rect_dag_pattern.col; j++){
	    dag_pattern.dag_pattern_finish[i * rect_dag_pattern.row + j] = 0; 	
	} 
    }
    //memset(dag_pattern.dag_pattern_finish,0,sizeof(int)*rect_dag_pattern.row*rect_dag_pattern.col);
    //dag_pattern.dag_pattern_head->next = NULL;
    if(dag_pattern_type == Left_Up_DAG)
    {
        for(i = 0; i < rect_dag_pattern.row ; i++)
        {
            for(j = 0; j < rect_dag_pattern.col; j++)
            {
                DAGElement dagelement;
                cnt = 2;
                if(i == rect_dag_pattern.row - 1)
                    cnt--;
                if(j == rect_dag_pattern.col - 1)
                    cnt--;
                dagelement.pos_cnt = cnt;
                dagelement.posfix_id = (int *)malloc(sizeof(int)*dagelement.pos_cnt);
                p = 0;
                if(i != rect_dag_pattern.row - 1)
                    dagelement.posfix_id[p++] = (i + 1) * rect_dag_pattern.col + j;
                if(j != rect_dag_pattern.col - 1)
                    dagelement.posfix_id[p++] = i * rect_dag_pattern.col + 1 + j;
            
                cnt = i + j;
                dagelement.data_pre_cnt = cnt;
                p = 0;
                dagelement.data_prefix_id = (int *)malloc(sizeof(int)*dagelement.data_pre_cnt);
                for(k = 0; k < i; k++)
                    dagelement.data_prefix_id[p++] = (i - 1 - k) * rect_dag_pattern.col + j;
                for(k = 0; k < j; k++)
                    dagelement.data_prefix_id[p++] = i * rect_dag_pattern.col + j - 1 - k;
                dagelement.pre_cnt = 2;
                if( i == 0 )
                    dagelement.pre_cnt--;
                if( j == 0 )
                    dagelement.pre_cnt--;
                dagelement.process = process;
                dag_pattern.dag_pattern_element[i * rect_dag_pattern.col + j] = dagelement;
            }
        }
    }
    /*
    int k;
    for(i = 0; i < rect_dag_pattern.row; i++)
    {
        for(j = 0; j < rect_dag_pattern.col; j++)
	{
	    int cc = rect_dag_pattern.row * i + j;
            int cnt = dag_pattern.dag_pattern_element[i * rect_dag_pattern.row + j].pos_cnt;
            printf("%d:",cc);
            for(k = 0; k < cnt; k++)
	    {
                printf(" %d",dag_pattern.dag_pattern_element[cc].posfix_id[k]);
	    }  
            printf("\n");
	}
    }*/
    SetDAGPattern(dag_pattern.dag_pattern_element,dag_pattern.dag_size,dag_pattern.block_size,dag_pattern.rect_size,dag_pattern.dag_pos,dag_pattern_type,dag_pattern.dag_pattern_finish);
    return;
}
