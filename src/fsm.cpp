#include <stdlib.h>

#include "fsm.h"

#define TRANSFER_SIZE 3
#define GROUP_ID 0x02

namespace LabCon {

FSM::FSM(void) : nrf(GROUP_ID), rxData(TRANSFER_SIZE), filter(A0), pc(USBTX, USBRX, 115200) {
    this->current_state = INIT;
    this->timeout_base_us = 50000;
    this->timeout_ts_us = 15 * 1000 * 1000;
    this->txDataCnt = 0;
    this->rxDataCnt = 0;
}

void FSM::spin(void) {
    switch (this->current_state) {
        case INIT: {
            this->nrf.begin(&(this->pc));
            this->filter.start();

            this->current_state = WAIT_BUZINA;

            break;
        }

        case WAIT_BUZINA: {
            if (this->filter.detected()) {
                this->txDataCnt = this->nrf.transmit(NRF::BUZINA_DET, 0x01);
                this->start_timer(this->timeout_base_us);

                this->current_state = WAIT_ACK;
            }

            break;
        }

        case WAIT_ACK: {
            if (this->nrf.readable()) {
                this->stop_timer();
                this->rxDataCnt = this->nrf.receive(&rxData);

                if (this->rxDataCnt == 3) {
                    if (this->rxData[0] == GROUP_ID && this->rxData[1] == NRF::BUZINA_DET) {
                        this->start_timer(this->timeout_ts_us);
                        this->current_state = WAIT_TS_TIMEOUT;
                    }
                }
            }

            if (this->did_timeout) {
                this->txDataCnt = this->nrf.transmit(NRF::BUZINA_DET, 0x01);
                this->reset_timer(this->timeout_base_us);

                this->current_state = WAIT_ACK;
            }

            break;
        }

        case WAIT_TS_TIMEOUT: {
            if (this->did_timeout) {
                this->stop_timer();
                this->filter.detected();
                this->current_state = WAIT_BUZINA;
            }

            break;
        }
    }
}

void FSM::start_timer(uint32_t t) {
    this->did_timeout = false;
    this->timeout.attach_us(callback(this, &FSM::_set_timeout), t + rand() % 50000);
}

void FSM::stop_timer() {
    this->timeout.detach();
}

void FSM::reset_timer(uint32_t t) {
    this->stop_timer();
    this->start_timer(t);
}

}  // namespace LabCon
