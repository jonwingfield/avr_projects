#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include "rht03.h"
#include "dbg.h"

#define RHT_INPUT_MODE()  RHT03_DDR  &= ~(1 << RHT03_DQ)
#define RHT_OUTPUT_MODE() RHT03_DDR  |=  (1 << RHT03_DQ)
#define RHT_LOW()         RHT03_PORT &= ~(1 << RHT03_DQ)
#define RHT_HIGH()        RHT03_PORT |=  (1 << RHT03_DQ)
#define RHT_IS_HIGH()         (RHT03_PIN & (1 << RHT03_DQ))

uint8_t U8FLAG, U8temp;

uint8_t  COM(void)
{
  uint8_t i, U8FLAG, U8comdata = 0;
  
  for(i=0;i<8;i++)           
  {

    U8FLAG=2;

    while((!RHT_IS_HIGH())&&U8FLAG++);
    _delay_us(10);
    _delay_us(10);
    _delay_us(10);
    U8temp=0;
    if(RHT_IS_HIGH())U8temp=1;
    U8FLAG=2;
    while((RHT_IS_HIGH())&&U8FLAG++);

    if(U8FLAG==1)break;
	            
    U8comdata<<=1;
    U8comdata|=U8temp;        //0
  }//rof

  return U8comdata;
}

bool rht03_temp_and_humidity(temp_humidity_info* info)
{
  uint8_t U8RH_data_H_temp, U8RH_data_L_temp, U8T_data_H_temp, U8T_data_L_temp, U8checkdata_temp;

  RHT_OUTPUT_MODE();
  RHT_LOW();
  _delay_ms(5);
  RHT_HIGH();
  _delay_us(10);
  _delay_us(10);
  RHT_INPUT_MODE();
  _delay_us(10);
  _delay_us(10);
  if(!RHT_IS_HIGH())                  
  {
    U8FLAG=2;
    while((!RHT_IS_HIGH())&&U8FLAG++);
    U8FLAG=2;
    while((RHT_IS_HIGH())&&U8FLAG++);
    U8RH_data_H_temp=COM();
    U8RH_data_L_temp=COM();
    U8T_data_H_temp=COM();
    U8T_data_L_temp=COM();
    U8checkdata_temp=COM();
    RHT_OUTPUT_MODE();
    RHT_HIGH();

    U8temp=(U8T_data_H_temp+U8T_data_L_temp+U8RH_data_H_temp+U8RH_data_L_temp);
    if(U8temp==U8checkdata_temp)
    {
  		info->temp = ((uint16_t)U8T_data_H_temp << 8) | U8T_data_L_temp;
      info->humidity = ((uint16_t)U8RH_data_H_temp << 8) | U8RH_data_L_temp;
          //    U8RH_data_H=U8RH_data_H_temp;
          //    U8RH_data_L=U8RH_data_L_temp;
          // U8T_data_H=U8T_data_H_temp;
          //    U8T_data_L=U8T_data_L_temp;
             // U8checkdata=U8checkdata_temp;
      return true;
    }
  }

  return false;
}

