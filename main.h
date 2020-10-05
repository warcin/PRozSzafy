
#ifndef GLOBALH
#define GLOBALH

#include <mpi.h>
#include <vector>
#include <stdio.h>
#include <windows.h>

//all 3 lines needed for visual studio compiling pthread
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#pragma comment(lib,"pthreadVC2.lib") 

#include "consts.h"
#include "structs.h"


extern Data datas; // full data set for one process

extern MPI_Datatype MPI_PACKET_T; // our 3 int packet defined in structs

// methods on mutex we share between comm and main threads for changing Data datas
void lockStateMutex();
void unlockStateMutex();

#endif