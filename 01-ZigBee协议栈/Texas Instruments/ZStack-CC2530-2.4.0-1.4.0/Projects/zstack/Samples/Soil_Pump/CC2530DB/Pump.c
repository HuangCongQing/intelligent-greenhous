#include "Pump.h"
#include <ioCC2530.h> 

void Pump_init(void)
{
  P0SEL &= ~0x02;          //P1.0Ϊ��ͨ I/O ��
  P0DIR |= 0x02;           //���
  
  P0_1 = 0;                //ˮ��Ĭ�Ϲر�
}


void Open_Pump(void)
{
  P0_1 = 1;
}


void Close_Pump(void)
{
  P0_1 = 0;
}