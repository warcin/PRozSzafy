#include "structs.h"
#include "main.h"
#include <stdio.h>

void incLamportTime(int received) {
    if (received > datas.lamportTime)
        datas.lamportTime = received;
    ++datas.lamportTime;
}

// returns true if we are having higher logical priority to resource
bool doWeHavePriority(int theirRank, int theirLamport, int ourLamport) {
    if ((theirLamport == ourLamport && theirRank > datas.rank) ||
        theirLamport > ourLamport)
        return true;
    return false;
}




void Data::init(int rank, int size) {
    this->state = State::INIT;
    this->rank = rank;
    this->size = size;
    this->roomDemand = 1; // POTRZEBNA FUNKCJA LOSUJACA
    this->knownRoomOccupancies.resize(size, -1);
    this->knownElevatorOccupancies.resize(size, -1);
    this->roomReservations.clear();
    this->elevatorReservations.clear();
    this->occupyingRoom = false;
    this->lamportTime = LAMPORT_DEFAULT;
    this->requestTime = LAMPORT_DEFAULT;

}

void Data::resetOccupancies()
{
    this->knownRoomOccupancies.clear();
    this->knownElevatorOccupancies.clear();
    this->knownRoomOccupancies.resize(size, -1);
    this->knownElevatorOccupancies.resize(size, -1);
}

void Data::broadcastCheckState(int type)
{
    packet_t* packet = (packet_t*)malloc(sizeof(packet_t));
    packet->lamportTime = datas.lamportTime;
    packet->resourceCount = 0;
    packet->resourceType = type;

    if (DEBUG) printf("broadcast CHECK_STATE(time = %d, resourceCount = %d, resourceType = %d) \n", packet->lamportTime, packet->resourceCount, packet->resourceType);
    for (int i = 0; i < datas.size; ++i) {
        if (i != datas.rank)
            MPI_Send(packet, 1, MPI_PACKET_T, i, Message::CHECK_STATE, MPI_COMM_WORLD);
    }
    
}

void Data::broadcastRelease(vector<int> recipents, int type)
{
    packet_t* packet = (packet_t*)malloc(sizeof(packet_t));
    packet->lamportTime = datas.lamportTime;
    packet->resourceCount = 0;
    packet->resourceType = type;

    if (DEBUG) printf("broadcast ANSWER_STATE(time = %d, resourceCount = %d, resourceType = %d) to %d recipents \n", packet->lamportTime, packet->resourceCount, packet->resourceType, recipents.size());
    for (int i = 0; i < recipents.size(); ++i) {
        if (recipents[i] != datas.rank)
            MPI_Send(packet, 1, MPI_PACKET_T, recipents[i], Message::ANSWER_STATE, MPI_COMM_WORLD);
    }
}


