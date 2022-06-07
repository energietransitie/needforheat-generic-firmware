#ifndef PROPERTY_FORMAT_H

// enum with all property types
typedef enum {
    PROPERTY_HEARTBEAT = 0,
    CO2_CONCENTRATION,
} property_t;

// Constants that tell how format data for each property type for sending to server
extern const char *format_property[];
extern const char *name_of_property[];

#endif