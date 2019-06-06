#if !defined(__NRF_H__)
#define __NRF_H__

#include "nRF24L01P.h"

namespace LabCon {

class NRF {
    public:
    struct rx_buffer_t {
        uint8_t* data;
        size_t size;

        rx_buffer_t(size_t size) {
            this->data = new uint8_t[size];
            this->size = size;
        }

        ~rx_buffer_t(void) { delete[] this->data; }

        uint8_t& operator[](int i) { return this->data[i]; }
    };

    enum msg_id_t {
        BUZINA_DET = 0x00,
    };

    NRF(uint8_t id, int transferSize = 3);

    void begin(Serial* serial = NULL);
    int transmit(msg_id_t msg_id, uint8_t data);
    int receive(rx_buffer_t* rxData);
    bool readable(void);

    private:
    nRF24L01P nrf;
    int transferSize;
    uint8_t id;
};

}  // namespace LabCon

#endif  // __NRF_H__
