#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <mpi.h>
#include "ScoringTable.h"
#include "Alignment.h"
#include "FastaLib.h"
#include "EasyHPS_src/EasyHPS.h"
#define SEQ_LEN 3000


int main(int argc, char** argv)
{
	int mypid,ret_level,node_num;
	MPI_Init_thread(&argc,&argv,MPI_THREAD_MULTIPLE,&ret_level);
	MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
	MPI_Comm_size(MPI_COMM_WORLD,&node_num);
	if (argc != 9)
	{
		fprintf(stderr, "USAGE: <appname> <score_table> \\ 
				<seq1> <seq2> <op> <ep> <len1> <len2> <thread_num>\n",argv[0]);
		exit(1);
	}
	int thread_sum = atoi(argv[8]);
	int thread_num = thread_sum / (node_num - 1);
	if(mypid <= thread_sum % (node_num - 1))
		thread_num++;
	Alignment_t alignment;
	alignment.op = atoi(argv[4]);
	alignment.ep = atoi(argv[5]);
	if(load_scoring_matrix(argv[1])!=0)
		exit(1);
	if(load_fasta_sequence(argv[2],&alignment.seq1,1) != 0)
		exit(1);
	if(load_fasta_sequence(argv[3],&alignment.seq2,1) != 0)
		exit(1);
	int m = atoi(argv[6]);
	int n = atoi(argv[7]);
	alignment.seq1.length = alignment.seq1.length < m ? alignment.seq1.length : m ;
	alignment.seq2.length = alignment.seq2.length < n ? alignment.seq2.length : n ;
	smith_waterman_init(&alignment);
	init_DAG_ptr_list();
	add_DAG_ptr_list((void*)get_score_values_pointer());
	add_DAG_ptr_list((void*)get_horizontal_gaps_pointer());
	add_DAG_ptr_list((void*)get_directions_pointer());
	add_DAG_ptr_list((void*)get_vertical_gaps_pointer());	
	SchedulerArgs masterSchedulerArgs,slaveSchedulerArgs;
	int timeout = 100;
	int block_size1 = 100;
	int block_size2 = 10;
	FILE *fp = fopen("EasyHPS_result.txt","w");
	SetMasterSchedulerArgs(0,0,get_DAG_ptr_list(),alignment.seq1.length+1,
			alignment.seq2.length+1,block_size1,node_num-1,timeout,Left_Up_DAG);
	SetSlaveSchedulerArgs(0,0,get_DAG_ptr_list(),block_size1,block_size2,
			thread_num,timeout,Left_Up_DAG);
	if(mypid == 0)
	{
		masterSchedulerArgs = GetMasterSchedulerArgs();
		EasyTHPS_Scheduler_Master(masterSchedulerArgs);
		smith_waterman_output(fp);
		fflush(fp);
		fclose(fp);
	}
	else
	{
		slaveSchedulerArgs = GetSlaveSchedulerArgs();
		slaveSchedulerArgs.process = smith_waterman_alignment;
		EasyTHPS_Scheduler_Slave(slaveSchedulerArgs);
	}
	MPI_Finalize();
	return 0;
}
