#ifndef _SCHEDULERSLAVE_H_
#define _SCHEDULERSLAVE_H_

#include "Scheduler.h"

void SetSlaveSchedulerArgs(int x,int y,DAG_data *data,int arg1,int arg2,int arg3,int timeout, enum DAG_pattern_type dag_pattern_type);

SchedulerArgs GetSlaveSchedulerArgs();

void EasyTHPS_Scheduler_Slave(SchedulerArgs schedulerArgs);

double get_thread_cal_time(int thread_num);
#endif
