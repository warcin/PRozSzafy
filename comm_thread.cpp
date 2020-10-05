#include "comm_thread.h"

void* commLoop(void* ptr) {
    MPI_Status status; // received message status
    packet_t packet; // received packet
    packet_t* response; // our response
    int senderLamportTime; // helper holding packet's lamport time
    int senderRank; // helper holding their rank

    //Communication loop
    while (true) {
        MPI_Recv(&packet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG) {

        // we send ANSWER_STATE to any CHECK_STATE with appropiate type and number of resources we hold / have priority to hold
        case Message::CHECK_STATE:
            if (DEBUG) printf("rank = %d, time = %d - received CHECK_STATE(time = %d, resourceCount = %d, resourceType = %d) from rank = %d \n", datas.rank, datas.lamportTime, packet.lamportTime, packet.resourceCount, packet.resourceType, status.MPI_SOURCE);

            
            // sender's lamport time and rank for priority check
            senderLamportTime = packet.lamportTime;
            senderRank = status.MPI_SOURCE;
            
            response = (packet_t*)malloc(sizeof(packet_t));
            response->resourceType = packet.resourceType; // answer's type is always the same as incoming packet's

            // update lamport time after receiving message
            lockStateMutex();
            incLamportTime(packet.lamportTime);
            unlockStateMutex();

            // Depending on state we are in, we answer how many rooms/elevators we hold onto or have priority to hold onto - if we respond with more than 0 in resourceCount we add them to appropiate release list to update them on that resource state
            if (packet.resourceType == Resource::ROOM) // switch-cases for room type
                switch (datas.state) {
                    case State::INIT:
                        response->resourceCount = 0;
                        break;
                    case State::IDLE:
                        response->resourceCount = 0;
                        break;
                    case State::IN_ELEVATOR:
                        if(datas.occupyingRoom) //our process is in elevator getting OUT of the room
                            response->resourceCount = 0;
                        else // we are coming to use our reserved rooms
                            response->resourceCount = datas.roomDemand;
                            //add them to release message list
                            lockStateMutex();
                            if (find(datas.roomReservations.begin(), datas.roomReservations.end(), status.MPI_SOURCE) == datas.roomReservations.end())
                                datas.roomReservations.push_back(status.MPI_SOURCE);
                            unlockStateMutex();
                        break;
                    case State::SEARCHING_FOR_ROOM: 
                        if (doWeHavePriority(senderRank, senderLamportTime, datas.requestTime)) {
                            response->resourceCount = datas.roomDemand;
                            //add them to release message list
                            lockStateMutex();
                            if (find(datas.roomReservations.begin(), datas.roomReservations.end(), status.MPI_SOURCE) == datas.roomReservations.end())
                                datas.roomReservations.push_back(status.MPI_SOURCE);
                            unlockStateMutex();
                        }
                        else
                            response->resourceCount = 0;
                        break;
                    default: // rest of states is when we are already reserving rooms and trying to get to them or actually using them
                        response->resourceCount = datas.roomDemand;
                        //add them to release message list
                        lockStateMutex();
                        if(find(datas.roomReservations.begin(),datas.roomReservations.end(),status.MPI_SOURCE) == datas.roomReservations.end())
                            datas.roomReservations.push_back(status.MPI_SOURCE);
                        unlockStateMutex();
                        break;
                }
            else // switch-cases for elevator type
                switch (datas.state) {
                    case State::IN_ELEVATOR:
                        response->resourceCount = 1;
                        //add them to release message list
                        lockStateMutex();
                        if (find(datas.elevatorReservations.begin(), datas.elevatorReservations.end(), status.MPI_SOURCE) == datas.elevatorReservations.end())
                            datas.elevatorReservations.push_back(status.MPI_SOURCE);
                        unlockStateMutex();
                        break;
                    case State::SEARCHING_FOR_ELEVATOR:
                        if (doWeHavePriority(senderRank, senderLamportTime, datas.requestTime)) {
                            response->resourceCount = 1;
                            //add them to release message list
                            lockStateMutex();
                            if (find(datas.elevatorReservations.begin(), datas.elevatorReservations.end(), status.MPI_SOURCE) == datas.elevatorReservations.end())
                                datas.elevatorReservations.push_back(status.MPI_SOURCE);
                            unlockStateMutex();
                        }
                        else
                            response->resourceCount = 0;
                        break;
                    default:
                        response->resourceCount = 0;
                        break;
                }

            // update lamport time (before sending)
            lockStateMutex();
            incLamportTime(LAMPORT_DEFAULT);
            response->lamportTime = datas.lamportTime;
            unlockStateMutex();

            // respond
            MPI_Send(response, 1, MPI_PACKET_T, status.MPI_SOURCE, Message::ANSWER_STATE, MPI_COMM_WORLD);
            if (DEBUG) printf("rank = %d, time = %d - send ANSWER_STATE(time = %d, resourceCount = %d, resourceType = %d) to rank = %d \n", datas.rank, datas.lamportTime, response->lamportTime, response->resourceCount, response->resourceType, status.MPI_SOURCE);
            free(response);

            break;

        // update our known global state vectors (knownRoomOccupancies and knownElevatorOccupancies) on incoming response / update
        case Message::ANSWER_STATE:
            if (DEBUG) printf("rank = %d, time = %d - received ANSWER_STATE(time = %d, resourceCount = %d, resourceType = %d)  from rank = %d \n", datas.rank, datas.lamportTime, packet.lamportTime, packet.resourceCount, packet.resourceType, status.MPI_SOURCE);

            lockStateMutex();
            // update lamport time
            incLamportTime(packet.lamportTime);
            // store information in list, check if it's for current request
            if(packet.lamportTime > datas.requestTime)
                if(packet.resourceType==Resource::ROOM)
                    datas.knownRoomOccupancies[status.MPI_SOURCE] = packet.resourceCount;
                else
                    datas.knownElevatorOccupancies[status.MPI_SOURCE] = packet.resourceCount;
            unlockStateMutex();

            break;
        default: // ignore tags that don't fit the program
            break;
        }
        fflush(stdout); //flush the printf's into console
    }
}