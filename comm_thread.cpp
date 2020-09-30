#include "comm_thread.h"

void* commLoop(void* ptr) {
    MPI_Status status;
    packet_t packet;

    while (true) {
        MPI_Recv(&packet, 1, MPI_PACKET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG) {
        case Message::CHECK_STATE:
            if (DEBUG) printf("received CHECK_STATE(time = %d, resourceCount = %d, resourceType = %d) from rank = %d \n", packet.lamportTime, packet.resourceCount, packet.resourceType, status.MPI_SOURCE);
            // variable for holding response lamport time at the end
            int lamportTime;
            // sender's lamport time for comparison
            int senderLamportTime = packet.lamportTime;
            int senderRank = status.MPI_SOURCE;

            packet_t *response = (packet_t*)malloc(sizeof(packet_t));
            response->resourceType = packet.resourceType;

            lockStateMutex();
            // update lamport time
            incLamportTime(packet.lamportTime);
            unlockStateMutex();
            if (packet.resourceType = Resource::ROOM)
                switch (datas.state) {
                    case State::INIT:
                        response->resourceCount = 0;
                        break;
                    case State::IDLE:
                        response->resourceCount = 0;
                        break;
                    case State::SEARCHING_FOR_ROOM: 
                        if (doWeHavePriority(senderRank, senderLamportTime, datas.requestTime))
                            response->resourceCount = datas.roomDemand;
                            //add them to release message list
                        else
                            response->resourceCount = 0;
                        break;
                    default:
                        response->resourceCount = datas.roomDemand;
                        //add them to release message list
                        break;
                }
            else
                switch (datas.state) {
                    case State::ELEVATOR:
                        response->resourceCount = 1;
                        //add them to release message list
                        break;
                    case State::SEARCHING_FOR_ELEVATOR:
                        if (doWeHavePriority(senderRank, senderLamportTime, datas.requestTime))
                            response->resourceCount = 1;
                            //add them to release message list
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
            MPI_Send(&response, 1, MPI_PACKET_T, status.MPI_SOURCE, Message::ANSWER_STATE, MPI_COMM_WORLD);
            if (DEBUG) printf("send ANSWER_STATE(time = %d, resourceCount = %d, resourceType = %d) to rank = %d \n", response->lamportTime, response->resourceCount, response->resourceType, status.MPI_SOURCE);

            break;
        case Message::ANSWER_STATE:
            if (DEBUG) printf("receive ANSWER_STATE(time = %d, resourceCount = %d, resourceType = %d)  from rank = %d \n", packet.lamportTime, packet.resourceCount, packet.resourceType, status.MPI_SOURCE);

            lockStateMutex();
            // update lamport time
            incLamportTime(packet.lamportTime);
            // store information in list
            datas.knownOccupancies[status.MPI_SOURCE] = packet.resourceCount;
            unlockStateMutex();

            break;
        default:
            break;
        }

    }
}