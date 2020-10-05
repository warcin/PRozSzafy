#ifndef STRUCTSH
#define STRUCTSH

#include <vector>

#include "consts.h"

using namespace std;

void incLamportTime(int received); // increments / updates lamportTime to the highest (received or our local) + 1
void updateRequestTime(int time);  // updates our data variable - see requestTime in Data

// communication is based around 3 integers sent in packets
typedef struct {
    int lamportTime;
    int resourceCount;
    int resourceType;
} packet_t;

// resourceType: 0 = Elevator, 1 = Room
enum Resource {
    ELEVATOR,
    ROOM
};

// states that process can be in
enum State {
    INIT,
    SEARCHING_FOR_ROOM,
    SEARCHING_FOR_ELEVATOR,
    IN_ELEVATOR,
    OCCUPYING,
    IDLE
};

// communication uses only two types (tags) of messages
enum Message {
    CHECK_STATE,
    ANSWER_STATE
};


// collection of all data used by one process
class Data {
public:
    State state;
    int rank, size; // our rank and size of whole communication world
    int lamportTime; // our current logical time
    int requestTime; // our logical time of current request for resource - used for resolving who has higher priority (it's time process uses to broadcast CHECK_STATE)
    int roomDemand; // how many rooms do we want for current state loop
    vector<int> knownRoomOccupancies; //both Occupancies vectors hold information about global state known to us
    vector<int> knownElevatorOccupancies;
    vector<int> roomReservations; //both Reservations vectors hold information who should we inform that we stopped using resource we had reserved / priority to reserve - used with broadcastRelease method
    vector<int> elevatorReservations;
    bool occupyingRoom; // information whether process wants to get in or out of the room

    void init(int rank, int size);
    void resetOccupancies(); // method to clear both vectors 
    void broadcastCheckState(int type); // method used to broadcast our need to get type of resource - sends appropiate CHECK_STATE
    void broadcastRelease(vector<int> recipents, int type); // method used to update other processes that might still want to get resource we reserved / had higher priority on - sends ANSWER_STATE with 0 as resourceCount

};

//helper method to resolve who has higher priority towards resource two processes compete to get - when the lamportTime is equal the process with lesser rank has priority
bool doWeHavePriority(int theirRank, int theirLamport, int ourLamport);

#endif