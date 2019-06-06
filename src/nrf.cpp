#include "nrf.h"

namespace LabCon {

NRF::NRF(nRF24L01P& nrf, int transferSize, uint8_t id) : nrf(nrf), transferSize(transferSize), id(id) {
    // transferSize must be 3 for now
    this->transferSize = 3;
}

void NRF::begin(Serial* serial) {
    this->nrf.powerUp();
    this->nrf.setTransferSize(this->transferSize);

    if (serial != NULL) {
        serial->printf("[NRF] Frequency    : %d MHz\r\n", this->nrf.getRfFrequency());
        serial->printf("[NRF] Out Power    : %d dBm\r\n", this->nrf.getRfOutputPower());
        serial->printf("[NRF] Data Rate    : %d kbps\r\n", this->nrf.getAirDataRate());
        serial->printf("[NRF] TX Addr.     : 0x%010llX\r\n", this->nrf.getTxAddress());
        serial->printf("[NRF] RX Addr.     : 0x%010llX\r\n", this->nrf.getRxAddress());
        serial->printf("[NRF] CRC          : %d bits\r\n", this->nrf.getCrcWidth());
        serial->printf("[NRF] Payload Size : %d bytes\r\n", this->nrf.getTransferSize());
    }

    this->nrf.setReceiveMode();
    this->nrf.enable();
}

int NRF::transmit(msg_id_t msg_id, uint8_t data) {
    uint8_t transferData[] = {this->id, uint8_t(msg_id), data};

    this->nrf.flushRx();
    return this->nrf.write(NRF24L01P_PIPE_P0, (char*) (transferData), this->transferSize);
}

int NRF::receive(rx_buffer_t* rxData) {
    return this->nrf.read(NRF24L01P_PIPE_P0, (char*) (rxData->data), rxData->size);
}

bool NRF::readable(void) {
    return this->nrf.readable();
}

}  // namespace LabCon
