#ifndef __temp_def_h__
#define __temp_def_h__

typedef struct 
{
	uint8_t sensor_id;
	uint16_t temperature_times_10;
	uint16_t humidity_times_10;
} weather_info;

#endif