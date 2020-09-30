#ifndef STRUCTSH
#define STRUCTSH

#include <vector>

#include "consts.h"

using namespace std;

typedef struct {
    int lamportTime;
    int resourceCount;
    int resourceType;
} packet_t;

enum Resource {
    ELEVATOR,
    ROOM
};

enum State {
    INIT,
    SEARCHING_FOR_ROOM,
    SEARCHING_FOR_ELEVATOR,
    ELEVATOR,
    OCCUPYING,
    IDLE
};

enum Message {
    CHECK_STATE,
    ANSWER_STATE
};

class Rooms {
public:
    vector<int> reservations;
};

class Elevators {
public:
    vector<int> reservations;
};

class Data {
public:
    State state;
    int rank, size;
    int lamportTime;
    int requestTime;
    int roomDemand;
    vector<int> knownOccupancies;
    bool occupyingRoom;
    Rooms rooms;
    Elevators elevators;

    void init(int rank, int size);
    void resetOccupancies();

};

bool doWeHavePriority(int theirRank, int theirLamport, int ourLamport);

#endif