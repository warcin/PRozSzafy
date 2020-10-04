#include "main_thread.h"
#include "main.h"
#include "structs.h"

void mainLoop() {
    if (DEBUG) printf("%d - DEBUG: ON \n", datas.rank);
    int perc;
    int occupyingTime;
    int potentialRooms;
    int potentialElevators;
    bool broadcasted = false;
    while (true){
        switch (datas.state) {
        case State::INIT:
            lockStateMutex();
            datas.state = State::IDLE;
            unlockStateMutex();
            break;
        case State::SEARCHING_FOR_ROOM:
            if (!broadcasted) {
                lockStateMutex();
                datas.resetOccupancies();
                incLamportTime(LAMPORT_DEFAULT);
                datas.broadcastCheckState(Resource::ROOM);
                datas.requestTime = datas.lamportTime;
                broadcasted = true;
                unlockStateMutex();
            }
            lockStateMutex();
            //potentially available rooms
            potentialRooms = ROOMS;
            for (int i = 0; i < datas.size; ++i)
                if (i != datas.rank)
                    if (datas.knownRoomOccupancies[i] == -1) //no data yet
                        potentialRooms -= MAX_ROOMS;
                    else
                        potentialRooms -= datas.knownRoomOccupancies[i];
            unlockStateMutex();
            if (potentialRooms >= datas.roomDemand) {
                if (DEBUG) printf("%d - Reserving %d rooms since there's at least %d left \n", datas.rank,datas.roomDemand,potentialRooms);
                lockStateMutex();
                datas.state = State::SEARCHING_FOR_ELEVATOR;
                broadcasted = false;
                unlockStateMutex();
            }
            break;
        case State::SEARCHING_FOR_ELEVATOR:
            if (!broadcasted) {
                lockStateMutex();
                datas.resetOccupancies();
                incLamportTime(LAMPORT_DEFAULT);
                datas.broadcastCheckState(Resource::ELEVATOR);
                datas.requestTime = datas.lamportTime;
                broadcasted = true;
                unlockStateMutex();
            }
            //potentially available elevators
            potentialElevators = ELEVATORS;
            for (int i = 0; i < datas.size; ++i)
                if (i != datas.rank)
                    if (datas.knownElevatorOccupancies[i] == -1) //no data yet
                        potentialElevators -= 1;
                    else
                        potentialElevators -= datas.knownElevatorOccupancies[i];

            if (potentialElevators >= 1) {
                if (DEBUG)printf("%d - Entering Elevator since there's at least %d left \n", datas.rank, potentialElevators);
                lockStateMutex();
                datas.state = State::IN_ELEVATOR;
                broadcasted = false;
                unlockStateMutex();
            }
            break;
        case State::IN_ELEVATOR:
            Sleep((SEC_IN_ELEVATOR - SEC_IN_STATE) * 1000);
            lockStateMutex();
            // process leaves the room
            if (datas.occupyingRoom) {
                datas.state = State::IDLE;
                datas.occupyingRoom = false;
            }
            // process starts occupying the room
            else {
                datas.state = State::OCCUPYING;
                datas.occupyingRoom = true;
            }
            incLamportTime(LAMPORT_DEFAULT);
            datas.broadcastRelease(datas.elevatorReservations, Resource::ELEVATOR);
            datas.elevatorReservations.clear();
            unlockStateMutex();
            
            break;
        case State::OCCUPYING:
            if (DEBUG)printf("%d - Using %d rooms  \n", datas.rank, datas.roomDemand);
            occupyingTime = rand() % (1 + MAX_SEC_IN_ROOM - MIN_SEC_IN_ROOM) + MIN_SEC_IN_ROOM;
            Sleep((occupyingTime - SEC_IN_STATE) * 1000);
            lockStateMutex();
            if (DEBUG)printf("%d - Left %d rooms  \n", datas.rank, datas.roomDemand);
            datas.state = State::SEARCHING_FOR_ELEVATOR;
            broadcasted = false;
            incLamportTime(LAMPORT_DEFAULT);
            datas.broadcastRelease(datas.roomReservations, Resource::ROOM);
            datas.roomReservations.clear();
            unlockStateMutex();
            break;
        case State::IDLE:
            perc = rand() % 100;
            if (perc < STATE_CHANGE_PROB) {
                lockStateMutex();
                datas.roomDemand = rand() % MAX_ROOMS + 1;
                datas.state = State::SEARCHING_FOR_ROOM;
                broadcasted = false;
                unlockStateMutex();
                if (DEBUG)printf("%d - Searching for %d rooms \n", datas.rank, datas.roomDemand);
            }
            break;
        default:
            break;
        }
        fflush(stdout);
        Sleep(SEC_IN_STATE*1000);
    }
}