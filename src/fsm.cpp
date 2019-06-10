#include <stdlib.h>

#include "fsm.h"

#define TRANSFER_SIZE 3
#define GROUP_ID 0x02

namespace LabCon {

FSM::FSM(void) : nrf(GROUP_ID), rxData(TRANSFER_SIZE), filter(A0), pc(USBTX, USBRX, 115200) {
    this->current_state = INIT;
    this->timeout_base_us = 75000;
    this->timeout_ts_us = 15 * 1000 * 1000;  // 15 seconds
    this->txDataCnt = 0;
    this->rxDataCnt = 0;
    this->retries = 0;
    this->count = 0;
}

void FSM::spin(void) {
    switch (this->current_state) {
        case INIT: {
            this->pc.printf("[FSM] [INIT] ENTER\r\n");
            this->nrf.begin(&(this->pc));

            this->txDataCnt = this->nrf.transmit(NRF::BUZINA_DET, 0x05, &(this->pc));

            // INIT -> WAIT_BUZINA transition
            this->pc.printf("[FSM] [transition] INIT -> WAIT_BUZINA\r\n");
            this->current_state = WAIT_BUZINA;

            this->pc.printf("[FIL] Starting...\r\n");
            this->filter.start();
            this->count++;

            break;
        }

        case WAIT_BUZINA: {
            // WAIT_BUZINA -> WAIT_ACK transition
            if (this->filter.detected()) {
                this->pc.printf("[FIL] Stoping...\r\n");
                this->filter.stop();
                this->txDataCnt = this->nrf.transmit(NRF::BUZINA_DET, this->count, &(this->pc));
                this->start_timer(this->timeout_base_us);
                this->pc.printf("[FSM] [WAIT_BUZINA] Transmitted %d bytes.\r\n", this->txDataCnt);

                this->pc.printf("[FSM] [transition] WAIT_BUZINA -> WAIT_ACK\r\n");
                this->retries = 0;
                this->current_state = WAIT_ACK;
            }

            break;
        }

        case WAIT_ACK: {
            if (this->nrf.readable()) {
                this->rxDataCnt = this->nrf.receive(&rxData);

                this->pc.printf("[FSM] [WAIT_ACK] Received %d bytes: [ 0x%02X, 0x%02X, 0x%02X ]\r\n", this->rxDataCnt,
                                this->rxData[0], this->rxData[1], this->rxData[2]);

                // WAIT_ACK -> WAIT_TS_TIMEOUT transition
                if (this->rxDataCnt == 3) {
                    if (this->rxData[0] == GROUP_ID /* && this->rxData[1] == NRF::BUZINA_DET */) {
                        this->stop_timer();
                        wait_ms(1);
                        this->start_timer(this->timeout_ts_us);

                        this->pc.printf("[FSM] [transition] WAIT_ACK -> WAIT_TS_TIMEOUT\r\n");
                        this->current_state = WAIT_TS_TIMEOUT;

                        break;
                    }
                }
            }

            if (this->retries > 15) {
                this->pc.printf("[FSM] [WAIT_ACK] Too many retries.\r\n");
                this->stop_timer();

                this->pc.printf("[FSM] [transition] WAIT_ACK -> WAIT_BUZINA\r\n");
                this->current_state = WAIT_BUZINA;

                this->pc.printf("[FIL] Starting...\r\n");
                this->filter.start();
                this->count++;

                break;
            }

            // WAIT_ACK -> WAIT_ACK transition
            if (this->did_timeout) {
                this->retries++;

                this->pc.printf("[FSM] [WAIT_ACK] Ack receive timed out.\r\n");
                this->txDataCnt = this->nrf.transmit(NRF::BUZINA_DET, this->count, &(this->pc));
                this->reset_timer(this->timeout_base_us);

                this->pc.printf("[FSM] [WAIT_ACK] Transmitted %d bytes.\r\n", this->txDataCnt);

                // this->pc.printf("[FSM] [transition] WAIT_ACK -> WAIT_ACK\r\n");
                this->current_state = WAIT_ACK;
            }

            break;
        }

        case WAIT_TS_TIMEOUT: {
            // WAIT_TS_TIMEOUT -> WAIT_BUZINA transition
            if (this->did_timeout) {
                this->stop_timer();
                this->filter.detected();

                this->pc.printf("[FSM] [transition] WAIT_TS_TIMEOUT -> WAIT_BUZINA\r\n");
                this->current_state = WAIT_BUZINA;

                this->pc.printf("[FIL] Starting...\r\n");
                this->filter.start();
                this->count++;
            }

            break;
        }
    }
}

void FSM::start_timer(uint32_t t) {
    this->did_timeout = false;
    this->timeout.attach_us(callback(this, &FSM::_set_timeout), t + rand() % 25000);
}

void FSM::stop_timer() {
    this->timeout.detach();
    wait_ms(1);
}

void FSM::reset_timer(uint32_t t) {
    this->stop_timer();
    this->start_timer(t);
}

}  // namespace LabCon
