#if !defined(__FILTER_H__)
#define __FILTER_H__

#include "mbed.h"

namespace LabCon {

class Filter {
    public:
    Filter(PinName ain_pin);

    void start(void);
    void stop(void);

    bool detected(bool reset = true);

    private:
    void process_sample(void);

    AnalogIn ain;
    Ticker sample_timer;

    bool _det;
    float _reading;

    // x(n), x(n - 1), x(n - 2)
    float x[3];
    float y1[3];
    float y2[3];
    float py1[3];
    float py2[3];
};

}  // namespace LabCon
#endif  // __FILTER_H__
