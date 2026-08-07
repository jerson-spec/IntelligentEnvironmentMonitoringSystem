#ifndef __DRIVER_OLED_H_
#define __DRIVER_OLED_H_

void OLED_Init(void);

void OLED_ShowChar(uint8_t x,uint8_t y,char content);
int OLED_ShowString(uint8_t x,uint8_t y,uint8_t* String);
int OLED_ShowDecimal(uint8_t x,uint8_t y,int32_t Number);

void OLED_Clear(void);	
void OLED_Test(void);

void OLEDTask(void* params);

#endif