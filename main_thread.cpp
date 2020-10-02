#include "main_thread.h"
#include "main.h"
#include "structs.h"

void mainLoop() {
    if (DEBUG) printf("DEBUG: ON \n");
    int perc;
    int occupyingTime;
    int potentialRooms;
    int potentialElevators;
    bool broadcasted = false;
    while (true) {


        switch (datas.state) {
        case State::INIT:
            lockStateMutex();
            datas.state = State::IDLE;
            unlockStateMutex();
            break;
        case State::SEARCHING_FOR_ROOM:
            if (!broadcasted) {
                lockStateMutex();
                incLamportTime(LAMPORT_DEFAULT);
                datas.broadcastCheckState(Resource::ROOM);
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
                lockStateMutex();
                datas.state = State::SEARCHING_FOR_ELEVATOR;
                broadcasted = false;
                unlockStateMutex();
            }
            break;
        case State::SEARCHING_FOR_ELEVATOR:
            if (!broadcasted) {
                lockStateMutex();
                incLamportTime(LAMPORT_DEFAULT);
                datas.broadcastCheckState(Resource::ELEVATOR);
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
            datas.broadcastRelease(datas.elevatorReservations, Resource::ELEVATOR);
            datas.elevatorReservations.clear();
            unlockStateMutex();
            
            break;
        case State::OCCUPYING:
            occupyingTime = rand() % (1 + MAX_SEC_IN_ROOM - MIN_SEC_IN_ROOM) + MIN_SEC_IN_ROOM;
            Sleep((occupyingTime - SEC_IN_STATE) * 1000);
            lockStateMutex();
            datas.state = State::SEARCHING_FOR_ELEVATOR;
            broadcasted = false;
            datas.broadcastRelease(datas.roomReservations, Resource::ROOM);
            datas.roomReservations.clear();
            unlockStateMutex();
            break;
        case State::IDLE:
            perc = rand() % 100;
            if (perc < STATE_CHANGE_PROB) {
                lockStateMutex();
                datas.state = State::SEARCHING_FOR_ROOM;
                broadcasted = false;
                unlockStateMutex();
            }
            break;
        default:
            break;
        }

        Sleep(SEC_IN_STATE*1000);
    }
}