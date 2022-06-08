#include "property_format.h"
#include "upload.h"

// list that tells for each property its format string
const char *format_property[] = {
    "%d", // PROPERTY_HEARTBEAT
    "%u", // CO2_CONCENTRATION
    "%.1f", // ROOM_TEMP_CO2
    "%.1f" // HUMIDITY
};

// list that tells for each property its name in string format
const char *name_of_property[] = {
    "heartbeat",    // PROPERTY_HEARTBEAT
    "CO2concentration", // CO2_CONCENTRATION
    "roomTempCO2", // ROOM_TEMP_CO2
    "relativeHumidity" // HUMIDITY
};

// functions that format value to string
void format_int(void *object, char *formated_value) {
    measurement_t *obj = (measurement_t *) object;
    sprintf(formated_value,format_property[obj->property],obj->value._int);
}

void format_uint16(void *object, char *formated_value) {
    measurement_t *obj = (measurement_t *) object;
    sprintf(formated_value,format_property[obj->property],obj->value._uint16);
}

void format_float(void *object, char *formated_value) {
    measurement_t *obj = (measurement_t *) object;
    sprintf(formated_value,format_property[obj->property],obj->value._float);
}

// list that tells for each property which function to use for formatting value to string
void (*const format_function_of_property[])(void *, char *) = {
    format_int,    // PROPERTY_HEARTBEAT
    format_uint16, // CO2_CONCENTRATION
    format_float, // ROOM_TEMP_CO2
    format_float // HUMIDITY
};