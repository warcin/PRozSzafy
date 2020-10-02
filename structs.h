#ifndef STRUCTSH
#define STRUCTSH

#include <vector>

#include "consts.h"

using namespace std;

void incLamportTime(int received);

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
    IN_ELEVATOR,
    OCCUPYING,
    IDLE
};

enum Message {
    CHECK_STATE,
    ANSWER_STATE
};



class Data {
public:
    State state;
    int rank, size;
    int lamportTime;
    int requestTime;
    int roomDemand;
    vector<int> knownRoomOccupancies;
    vector<int> knownElevatorOccupancies;
    vector<int> roomReservations;
    vector<int> elevatorReservations;
    bool occupyingRoom; // information whether process wants to get in or out of the room

    void init(int rank, int size);
    void resetOccupancies();
    void broadcastCheckState(int type);
    void broadcastRelease(vector<int> recipents, int type);

};

bool doWeHavePriority(int theirRank, int theirLamport, int ourLamport);

#endif