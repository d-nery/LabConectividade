#include "mbed.h"

#include "fsm.h"

using namespace LabCon;

int main() {
    srand(236983);

    FSM fsm;

    for (;;) {
        fsm.spin();
    }
}
