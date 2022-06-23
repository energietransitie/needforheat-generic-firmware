#ifndef PROPERTY_FORMAT_H
#define PROPERTY_FORMAT_H

// enum with all property types
typedef enum {
    PROPERTY_HEARTBEAT = 0,
    CO2_CONCENTRATION,
    ROOM_TEMP_CO2,
    HUMIDITY,
    PROPERTY_BATTERY_VOLTAGE
} property_t;

// Constants that tell how format data for each property type for sending to server
extern const char *format_property[];
extern const char *name_of_property[];
extern void (* const format_function_of_property[])(void *, char *);

#endif