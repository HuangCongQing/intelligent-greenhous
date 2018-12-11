#include "Heater.h"
#include <ioCC2530.h> 

void Heater_init(void)
{
  P0SEL &= ~0x02;          //P1.0Ϊ��ͨ I/O ��
  P0DIR |= 0x02;           //���
  
  P0_1 = 0;                //ˮ��Ĭ�Ϲر�
}


void Open_Heater(void)
{
  P0_1 = 1;
}


void Close_Heater(void)
{
  P0_1 = 0;
}