#include "Fan.h"
#include <ioCC2530.h> 

void Fan_init(void)
{
  P0SEL &= ~0x02;          //P1.0Ϊ��ͨ I/O ��
  P0DIR |= 0x02;           //���
  
  P0_1 = 0;                //����Ĭ�Ϲر�
}


void Open_Fan(void)
{
  P0_1 = 1;
}


void Close_Fan(void)
{
  P0_1 = 0;
}