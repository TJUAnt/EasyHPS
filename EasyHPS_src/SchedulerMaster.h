#ifndef _SCHEDULERMASTER_H_
#define _SCHEDULERMASTER_H_

#include "Scheduler.h"

void EasyTHPS_Scheduler_Master(SchedulerArgs schedulerArgs);

SchedulerArgs GetMasterSchedulerArgs();

void SetMasterSchedulerArgs(int x,int y,DAG_data* data,int dag_row,int dag_col,int arg2,int arg3,int timeout, enum DAG_pattern_type dag_pattern_type);



#endif
