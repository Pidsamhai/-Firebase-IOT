#include "Arduino.h"

struct Config
{
public:
    /* Time in miliseconds */
    unsigned long DISPLAY_INTERVAL;
    /* Time in miliseconds */
    unsigned long UPLOAD_DATA_INTERVAL;

    void init();

    void save();

    void parse(byte *playload);
};