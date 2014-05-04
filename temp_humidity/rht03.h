#ifndef __rht03_h__
#define __rht03_h__

#define RHT03_PORT PORTD
#define RHT03_DDR  DDRD
#define RHT03_PIN  PIND
#define RHT03_DQ   PD3

typedef struct 
{
	uint16_t temp;
	uint16_t humidity;
} temp_humidity_info;

/* reads temp/humidity info from RHT_03 and populates the struct provided */
bool rht03_temp_and_humidity(temp_humidity_info* info);

#endif