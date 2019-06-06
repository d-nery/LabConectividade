#include "mbed.h"
#include "nRF24L01P.h"

#include "filter.h"
#include "nrf.h"

#define TRANSFER_SIZE 3
#define GROUP_ID 0x02

using namespace LabCon;

DigitalOut led1(LED1);
Serial pc(USBTX, USBRX, 115200);

Timeout timer;
nRF24L01P nrf24l01p(PA_7, PA_6, PB_3, PA_9, PC_7, PA_8);  // mosi, miso, sck, csn, ce, irq

Filter filter(A0);
NRF nrf(nrf24l01p, TRANSFER_SIZE, GROUP_ID);

bool timeout = false;

typedef enum {
    INIT,
    WAIT_BUZINA,
    WAIT_ACK,
    WAIT_TS_TIMEOUT,
} states_t;

void start_timer(uint32_t t) {
    timer.attach_us([](void) -> void { timeout = true; }, t + rand() % 50000);
}

void stop_timer() {
    timer.detach();
}

void reset_timer(uint32_t t) {
    stop_timer();
    start_timer(t);
}

int main() {
    NRF::rx_buffer_t rxData(TRANSFER_SIZE);

    int txDataCnt = 0;  // Bytes enviados
    int rxDataCnt = 0;  // Bytes recebidos

    states_t current_state = INIT;

    uint32_t timeout_base_us = 50000;
    uint32_t timeout_ts_us = 15 * 1000 * 1000;

    srand(236983);

    for (;;) {
        switch (current_state) {
            case INIT: {
                nrf.begin(&pc);
                filter.start();

                current_state = WAIT_BUZINA;

                break;
            }

            case WAIT_BUZINA: {
                if (filter.detected()) {
                    txDataCnt = nrf.transmit(NRF::BUZINA_DET, 0x01);
                    start_timer(timeout_base_us);

                    current_state = WAIT_ACK;
                }

                break;
            }

            case WAIT_ACK: {
                if (nrf.readable()) {
                    stop_timer();
                    rxDataCnt = nrf.receive(&rxData);

                    if (rxDataCnt == 3) {
                        if (rxData[0] == GROUP_ID && rxData[1] == NRF::BUZINA_DET) {
                            start_timer(timeout_ts_us);
                            current_state = WAIT_TS_TIMEOUT;
                        }
                    }
                }

                if (timeout) {
                    timeout = false;

                    txDataCnt = nrf.transmit(NRF::BUZINA_DET, 0x01);
                    reset_timer(timeout_base_us);

                    current_state = WAIT_ACK;
                }

                break;
            }

            case WAIT_TS_TIMEOUT: {
                if (timeout) {
                    stop_timer();
                    filter.detected();
                    current_state = WAIT_BUZINA;
                }

                break;
            }
        }
    }
}
