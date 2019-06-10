#if !defined(__FSM_H__)
#define __FSM_H__

#include "mbed.h"

#include "filter.h"
#include "nrf.h"

namespace LabCon {

class FSM {
    public:
    enum state_t {
        INIT,
        WAIT_BUZINA,
        WAIT_ACK,
        WAIT_TS_TIMEOUT,
    };

    FSM();
    void spin(void);

    private:
    Timeout timeout;

    NRF nrf;
    NRF::rx_buffer_t rxData;

    Filter filter;
    Serial pc;

    int txDataCnt;  // Bytes enviados
    int rxDataCnt;  // Bytes recebidos

    bool did_timeout;
    state_t current_state;

    uint32_t timeout_base_us;
    uint32_t timeout_ts_us;

    uint32_t retries;
    uint8_t count;

    void start_timer(uint32_t t);
    void stop_timer(void);
    void reset_timer(uint32_t t);

    void _set_timeout(void) { did_timeout = true; };
};

}  // namespace LabCon

#endif  // __FSM_H__
