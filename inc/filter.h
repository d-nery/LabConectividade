#if !defined(__FILTER_H__)
#define __FILTER_H__

class Filter {
    public:
        Filter(PinName ain_pin);

        void start(void);
        void stop(void);

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

#endif // __FILTER_H__
