#include "main_thread.h"
#include "main.h"
#include "structs.h"

void mainLoop() {
    if (DEBUG) printf("DEBUG: ON \n");

    while (true) {
        int perc = rand() % 100;
        int walkTime;

        switch (datas.state) {
        case State::INIT:
            break;
        case State::SEARCHING_FOR_ROOM:
            break;
        case State::SEARCHING_FOR_ELEVATOR:
            break;
        case State::ELEVATOR:
            break;
        case State::OCCUPYING:
            break;
        case State::IDLE:
            break;
        default:
            break;
        }

        Sleep(SEC_IN_STATE*1000);
    }
}