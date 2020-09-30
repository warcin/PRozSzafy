#ifndef COMM_THREADH
#define COMM_THREADH

#include <mpi.h>

#include "consts.h"
#include "structs.h"
#include "main.h"

void* commLoop(void* ptr);

#endif