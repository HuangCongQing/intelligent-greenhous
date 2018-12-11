/******************************************************************************
* INCLUDES
*/
#include "CO2.h"

void CO2_init(void)
{
  P0SEL |= 0x01;
  ADCCON3  = (0xB0);                    //ѡ��AVDD5Ϊ�ο���ѹ��12�ֱ��ʣ�P0_0  ADC
  
  ADCCON1 |= 0x30;                      //ѡ��ADC������ģʽΪ�ֶ�
}

float GetADC(void)
{
  float ret = 0.0f;
  unsigned int  value;
  P0SEL |= 0x01;
  ADCCON3  = (0xB0);                    //ѡ��AVDD5Ϊ�ο���ѹ��12�ֱ��ʣ�P0_0  ADC
  
  ADCCON1 |= 0x30;                      //ѡ��ADC������ģʽΪ�ֶ�
  ADCCON1 |= 0x40;                      //����ADת��             
  
  while(!(ADCCON1 & 0x80));             //�ȴ�ADCת������
  
  value =  ADCL >> 2;
  value |= (ADCH << 6);                 //ȡ������ת�����������value��
  ret = (float)(value / 4.0);
  return ret;  
}

