#include "comm_thread.h"

void* commLoop(void* ptr) {
    MPI_Status status;
    packet_t packet;
    packet_t* response;
    int senderLamportTime;
    int senderRank;
    int lamportTime;
    while (true) {
        MPI_Recv(&packet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG) {
        case Message::CHECK_STATE:
            if (DEBUG) printf("%d - received CHECK_STATE(time = %d, resourceCount = %d, resourceType = %d) from rank = %d \n", datas.rank, packet.lamportTime, packet.resourceCount, packet.resourceType, status.MPI_SOURCE);
            // variable for holding response lamport time at the end
            
            // sender's lamport time for comparison
            senderLamportTime = packet.lamportTime;
            senderRank = status.MPI_SOURCE;
            
            response = (packet_t*)malloc(sizeof(packet_t));
            response->resourceType = packet.resourceType;

            lockStateMutex();
            // update lamport time
            incLamportTime(packet.lamportTime);
            unlockStateMutex();
            if (packet.resourceType == Resource::ROOM)
                switch (datas.state) {
                    case State::INIT:
                        response->resourceCount = 0;
                        break;
                    case State::IDLE:
                        response->resourceCount = 0;
                        break;
                    case State::IN_ELEVATOR:
                        if(datas.occupyingRoom) //process is in elevator getting OUT of the room
                            response->resourceCount = 0;
                        else
                            response->resourceCount = datas.roomDemand;
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
                    default:
                        response->resourceCount = datas.roomDemand;
                        //add them to release message list
                        lockStateMutex();
                        if(find(datas.roomReservations.begin(),datas.roomReservations.end(),status.MPI_SOURCE) == datas.roomReservations.end())
                            datas.roomReservations.push_back(status.MPI_SOURCE);
                        unlockStateMutex();
                        break;
                }
            else
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
            lamportTime = datas.lamportTime;
            unlockStateMutex();

           
            
            response->lamportTime = lamportTime;

            // respond
            MPI_Send(response, 1, MPI_PACKET_T, status.MPI_SOURCE, Message::ANSWER_STATE, MPI_COMM_WORLD);
            if (DEBUG) printf("%d - send ANSWER_STATE(time = %d, resourceCount = %d, resourceType = %d) to rank = %d \n", datas.rank, response->lamportTime, response->resourceCount, response->resourceType, status.MPI_SOURCE);
            free(response);

            break;
        case Message::ANSWER_STATE:
            if (DEBUG) printf("%d - received ANSWER_STATE(time = %d, resourceCount = %d, resourceType = %d)  from rank = %d \n", datas.rank, packet.lamportTime, packet.resourceCount, packet.resourceType, status.MPI_SOURCE);

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
        default:
            break;
        }
        fflush(stdout);
    }
}