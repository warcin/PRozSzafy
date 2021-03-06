﻿#include "main.h"
#include "comm_thread.h"
#include "main_thread.h"


MPI_Datatype MPI_PACKET_T;

Data datas;

pthread_t commThread;
pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;

// lawlessly stolen from project skeleton
void check_thread_support(int provided) {
    if (DEBUG) printf("rank = %d- THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", datas.rank, provided);
    switch (provided) {
    case MPI_THREAD_SINGLE:
        if (DEBUG) printf("Brak wsparcia dla wątków, kończę\n");
        fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
        MPI_Finalize();
        exit(-1);
        break;
    case MPI_THREAD_FUNNELED:
        if (DEBUG) printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
        break;
    case MPI_THREAD_SERIALIZED:
        if (DEBUG) printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
        break;
    case MPI_THREAD_MULTIPLE:
        if (DEBUG) printf("Pełne wsparcie dla wątków\n");
        break;
    default:
        if (DEBUG) printf("Nikt nic nie wie\n");
    }
}

//preparing MPI packet from ours defined in structs
void initType() {
    const int nItems = 3;
    int blockLengths[nItems] = { 1, 1, 1 };
    MPI_Datatype types[nItems] = { MPI_INT, MPI_INT, MPI_INT };

    MPI_Aint offsets[nItems];
    offsets[0] = offsetof(packet_t, lamportTime);
    offsets[1] = offsetof(packet_t, resourceCount);
    offsets[2] = offsetof(packet_t, resourceType);

    MPI_Type_create_struct(nItems, blockLengths, offsets, types, &MPI_PACKET_T);
    MPI_Type_commit(&MPI_PACKET_T);
}

// every initialisation in one function
void initApp(int* argc, char*** argv) {
    if (DEBUG) printf("rank = %d - Application initialization...\n", datas.rank);
    int provided;

    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);

    initType();

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(time(NULL) * rank);
    datas.init(rank, size);

    pthread_create(&commThread, NULL, commLoop, 0);

    Sleep(1000);
    if (DEBUG) printf("rank = %d - Application ready \n", datas.rank);
}

// close everything
void finalizeApp() {
    pthread_join(commThread, NULL);
    MPI_Type_free(&MPI_PACKET_T);
    MPI_Finalize();
}



void lockStateMutex() {
    pthread_mutex_lock(&stateMutex);
}

void unlockStateMutex() {
    pthread_mutex_unlock(&stateMutex);
}

// heart of the program
int main(int argc, char** argv) {
    initApp(&argc, &argv);
    mainLoop();
    finalizeApp();
    return 0;
}