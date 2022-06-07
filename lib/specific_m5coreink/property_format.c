#include "property_format.h"

// list that tells for each property how format data with sprintf
const char *format_property[] = {
    "%d", // PROPERTY_HEARTBEAT
    "%d", // CO2_CONCENTRATION
};

// list that tells for each property its name in string format
const char *name_of_property[] = {
    "heartbeat",    // PROPERTY_HEARTBEAT
    "CO2concentration" // CO2_CONCENTRATION
};