#include "mbed.h"
#include "nRF24L01P.h"

#include "filter.h"
#include "mcu.h"

#define TRANSFER_SIZE   3

DigitalOut led1(LED1);
Serial pc(USBTX, USBRX, 115200);

Timeout timer;
nRF24L01P nrf(PA_7, PA_6, PB_3, PA_9, PC_7, PA_8);    // mosi, miso, sck, csn, ce, irq
Filter filter(A0);

char txData[3] = { 0 };
bool timeout = false;

typedef enum {
    WAIT_TX0,
    WAIT_ACK0,
    WAIT_TX1,
    WAIT_ACK1,
} states_t;

int transmit() {
    nrf.flushRx();
    pc.printf("Valores transmitidos: %d, %d, %d\r\n", txData[0],txData[1],txData[2]);
    return nrf.write(NRF24L01P_PIPE_P0, txData, sizeof(txData));
}

void did_timeout() {
    timeout = true;
}

void start_timer(uint32_t t) {
    timer.attach_us(&did_timeout, t + rand() % 50000);
}

void stop_timer() {
    timer.detach();
}

void reset_timer(uint32_t t) {
    stop_timer();
    start_timer(t);
}


int main() {
    // Num de bytes tx = num de bytes rx
    char rxData[TRANSFER_SIZE];
    int txDataCnt = 0;   //Bytes enviados
    int rxDataCnt = 0;   //Bytes recebidos

    nrf.powerUp();   //Aciona a NRF24L01+
    nrf.setTransferSize( TRANSFER_SIZE );  //Determina tamanho do payload

    // Mostre as configurações
    pc.printf( "nRF24L01+ Frequencia    : %d MHz\r\n",  nrf.getRfFrequency() );
    pc.printf( "nRF24L01+ Pot. de saida : %d dBm\r\n",  nrf.getRfOutputPower() );
    pc.printf( "nRF24L01+ Taxa          : %d kbps\r\n", nrf.getAirDataRate() );
    pc.printf( "nRF24L01+ End. TX       : 0x%010llX\r\n", nrf.getTxAddress() );
    pc.printf( "nRF24L01+ End. RX       : 0x%010llX\r\n", nrf.getRxAddress() );
    pc.printf( "nRF24L01+ CRC           : %d bits\r\n", nrf.getCrcWidth() );
    pc.printf( "nRF24L01+ Tam. Payload  : %d bytes\r\n", nrf.getTransferSize() );

    srand(236983);

    nrf.setReceiveMode();
    nrf.enable();

    states_t current_state = WAIT_TX0;
    //states_t next_state = WAIT_TX0;

    uint32_t cont = 0;

    uint32_t timeout_base_us = 50000;

    bool mensagem_valida = false;

    //uint16_t value = 888;
    //char api_key[] = "4WO1SUY3GPQXSJPW";
    //char header[1024];

    filter.start();
    // timer_led.attach(&toggle_led, 0.5);

    while (1) {
        txData[0] = 0x02;

        switch (current_state) {
            case WAIT_TX0: {
                while(led1==0);
                txData[1] = 0x00;
                txData[2] = cont;

                txDataCnt = transmit();
                pc.printf("transmitilll\r\n");
                start_timer(timeout_base_us);

                current_state = WAIT_ACK0;


                break;
            }

            case WAIT_ACK0: {

                mensagem_valida = false;

                while(!mensagem_valida) {

                    while (!nrf.readable() && !timeout);

                    if (timeout) {
                        timeout = false;
                        pc.printf("Timeout recebido seq0, retransmitindo...\r\n");
                        transmit();
                        reset_timer(timeout_base_us);
                        continue;
                    }

                    stop_timer();
                    rxDataCnt = nrf.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
                    pc.printf("Valores recebidos: %d, %d, %d\r\n", rxData[0],rxData[1],rxData[2]);

                    if (rxDataCnt == 3) {
                        if (rxData[0] == 0x02 && rxData[1] == 0x00) {
                            cont++;
                            current_state = WAIT_TX1;

                            mensagem_valida = true;

                            continue;
                        }
                    }
                }
            }

            case WAIT_TX1: {
                while(led1==0);
                txData[1] = 0x01;
                txData[2] = cont;

                nrf.flushRx();
                txDataCnt = transmit();
                start_timer(timeout_base_us);

                current_state = WAIT_ACK1;


                break;
            }
            case WAIT_ACK1: {

                bool mensagem_valida = false;

                while(!mensagem_valida) {

                    while (!nrf.readable() && !timeout);

                    if (timeout) {
                        timeout = false;
                        pc.printf("Timeout recebido seq1, retransmitindo...\r\n");
                        transmit();
                        reset_timer(timeout_base_us);
                        continue;
                    }

                    stop_timer();
                    rxDataCnt = nrf.read( NRF24L01P_PIPE_P0, rxData, sizeof( rxData ) );
                    pc.printf("Valores recebidos: %d, %d, %d\r\n", rxData[0],rxData[1],rxData[2]);

                    if (rxDataCnt == 3) {
                        if (rxData[0] == 0x02 && rxData[1] == 0x01) {
                            cont++;
                            current_state = WAIT_TX0;

                            mensagem_valida = true;

                            continue;
                        }
                    }
                }
            }
        }
    }
}
