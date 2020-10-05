#include "main_thread.h"
#include "main.h"
#include "structs.h"

void mainLoop() {
    if (DEBUG) printf("rank = %d, time = %d - DEBUG: ON \n", datas.rank, datas.lamportTime);
    int perc; // helper used for randomizing need to get into room when process is in IDLE state
    int occupyingTime; // same as above but for time we want to spend in room
    int potentialRooms; // for calculating currently available rooms based on responses we got so far
    int potentialElevators; // as above but for elevators
    bool broadcasted = false; // we only broadcast messages ONCE

    //Main loop
    while (true){
        switch (datas.state) {
        // we aren't using INIT state for anything, but we could have
        case State::INIT:
            lockStateMutex();
            datas.state = State::IDLE;
            unlockStateMutex();
            break;

        // Broadcast and then calculate possibility of room reservation based on replies we get
        case State::SEARCHING_FOR_ROOM:
            if (!broadcasted) {
                lockStateMutex();
                datas.resetOccupancies();
                incLamportTime(LAMPORT_DEFAULT);
                updateRequestTime(datas.lamportTime);
                if (DEBUG)printf("rank = %d, time = %d - Searching for %d rooms, time = %d \n", datas.rank, datas.lamportTime, datas.roomDemand, datas.requestTime);
                datas.broadcastCheckState(Resource::ROOM);
                broadcasted = true;
                unlockStateMutex();
            }

            //Calculate the possibility of entry
            potentialRooms = ROOMS;
            for (int i = 0; i < datas.size; ++i)
                if (i != datas.rank)
                    if (datas.knownRoomOccupancies[i] == -1) //no data yet
                        potentialRooms -= MAX_ROOMS;
                    else
                        potentialRooms -= datas.knownRoomOccupancies[i];

            if (potentialRooms >= datas.roomDemand) {
                if (DEBUG) printf("rank = %d, time = %d - Reserving %d rooms since there's at least %d left \n", datas.rank, datas.lamportTime, datas.roomDemand,potentialRooms);
                lockStateMutex();
                datas.state = State::SEARCHING_FOR_ELEVATOR;
                broadcasted = false;
                unlockStateMutex();
            }
            break;
        // Broadcast and then calculate possibility of entering the elevator based on replies we get
        case State::SEARCHING_FOR_ELEVATOR:
            if (!broadcasted) {
                lockStateMutex();
                datas.resetOccupancies();
                incLamportTime(LAMPORT_DEFAULT);
                updateRequestTime(datas.lamportTime);
                if (DEBUG) printf("rank = %d, time = %d - Searching for elevator, time = %d \n", datas.rank, datas.lamportTime, datas.requestTime);
                datas.broadcastCheckState(Resource::ELEVATOR);
                broadcasted = true;
                unlockStateMutex();
            }

            //Calculate the possibility of entry
            potentialElevators = ELEVATORS;
            for (int i = 0; i < datas.size; ++i)
                if (i != datas.rank)
                    if (datas.knownElevatorOccupancies[i] == -1) //no data yet
                        potentialElevators -= 1;
                    else
                        potentialElevators -= datas.knownElevatorOccupancies[i];

            if (potentialElevators >= 1) {
                if (DEBUG)printf("rank = %d, time = %d - Entering Elevator since there's at least %d left \n", datas.rank, datas.lamportTime, potentialElevators);
                lockStateMutex();
                datas.state = State::IN_ELEVATOR;
                broadcasted = false;
                unlockStateMutex();
            }
            break;

        //Idle in elevator (either TO or OUT of the room) - broadcast release if needed
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

        // Idling in room(s) - broadcast release if needed
        case State::OCCUPYING:
            if (DEBUG)printf("rank = %d, time = %d - Using %d rooms  \n", datas.rank, datas.lamportTime, datas.roomDemand);
            occupyingTime = rand() % (1 + MAX_SEC_IN_ROOM - MIN_SEC_IN_ROOM) + MIN_SEC_IN_ROOM;
            Sleep((occupyingTime - SEC_IN_STATE) * 1000);
            lockStateMutex();
            if (DEBUG)printf("rank = %d, time = %d - Left %d rooms  \n", datas.rank, datas.lamportTime, datas.roomDemand);
            datas.state = State::SEARCHING_FOR_ELEVATOR;
            broadcasted = false;
            incLamportTime(LAMPORT_DEFAULT);
            datas.broadcastRelease(datas.roomReservations, Resource::ROOM);
            datas.roomReservations.clear();
            unlockStateMutex();
            break;

        // Idling outside, randomly getting need for room(s)
        case State::IDLE:
            perc = rand() % 100;
            if (perc < STATE_CHANGE_PROB) {
                lockStateMutex();
                datas.roomDemand = rand() % MAX_ROOMS + 1;
                datas.state = State::SEARCHING_FOR_ROOM;
                broadcasted = false;
                unlockStateMutex();
                
            }
            break;
        default:
            break;
        }
        fflush(stdout); //flush the printf's into console
        Sleep(SEC_IN_STATE*1000);
    }
}