#include "structs.h"
#include "main.h"
#include <stdio.h>



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
    this->knownOccupancies.resize(size, -1);
    this->occupyingRoom = false;
    this->lamportTime = LAMPORT_DEFAULT;
    this->requestTime = LAMPORT_DEFAULT;

}

void Data::resetOccupancies()
{
    this->knownOccupancies.clear();
    this->knownOccupancies.resize(size, -1);
}


