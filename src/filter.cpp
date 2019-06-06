#include "filter.h"
#include "mbed.h"

static const float lim = 0.6f;
static const float b1[] = {0.0233702166f, 0.0f, -0.0233702166f};
static const float b2[] = {0.0616017234f, 0.0f, -0.0616017234f};
static const float a1[] = {1.0f, -1.5339278083f, 0.9532595669f};
static const float a2[] = {1.0f, -1.4738801091f, 0.8767965533f};

namespace LabCon {

Filter::Filter(PinName ain_pin) : ain(ain_pin) {
    this->_det = false;
}

void Filter::start(void) {
    this->sample_timer.attach(callback(this, &Filter::process_sample), 0.000125);
}

void Filter::stop(void) {
    this->sample_timer.detach();
}

bool Filter::detected(bool reset) {
    if (!this->_det) {
        return false;
    }

    if (reset) {
        this->_det = false;
    }

    return true;
}

void Filter::process_sample(void) {
    this->_reading = this->ain.read() - 0.5f;

    x[2] = x[1];
    x[1] = x[0];
    x[0] = this->_reading;

    y1[2] = y1[1];
    y1[1] = y1[0];

    y2[2] = y2[1];
    y2[1] = y2[0];

    // filtro 1
    y1[0] = b1[0] * x[0] + b1[1] * x[1] + b1[2] * x[2] - a1[1] * y1[1] - a1[2] * y1[2];

    // filtro 2
    y2[0] = b2[0] * x[0] + b2[1] * x[1] + b2[2] * x[2] - a2[1] * y2[1] - a2[2] * y2[2];

    // Comparacao com limiar
    py1[1] = py1[0];
    py1[0] = 0.001f * y1[0] * y1[0] + 0.999f * py1[1];

    py2[1] = py2[0];
    py2[0] = 0.001f * y2[0] * y2[0] + 0.999f * py2[1];

    if (py1[0] > (lim * py2[0])) {
        this->_det = true;
    }
}

}  // namespace LabCon
