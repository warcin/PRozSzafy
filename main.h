
#ifndef GLOBALH
#define GLOBALH

#include <mpi.h>
#include <vector>
#include <stdio.h>
#include <windows.h>

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#pragma comment(lib,"pthreadVC2.lib") //for visual studio

#include "consts.h"
#include "structs.h"

//#define println(FORMAT, ...) printf("%c[%d;%dm[r = %d][t = %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(datass.rank/7))%2, 31+(6+datass.rank)%7, datass.rank, datass.lamportTime, ##__VA_ARGS__, 27,0,37);

extern Data datas; // full datas set for one process

extern MPI_Datatype MPI_PACKET_T;


void lockStateMutex();
void unlockStateMutex();

#endif