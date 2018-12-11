/********************************************************************************
* Copyright (c) 2015�������пأ��������Ƽ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�SHT10.h
* ժ	Ҫ��
*
* ��ǰ�汾��V1.0
* ��	�ߣ���С��
* ������ڣ�2016��3��31��
* �޸�ժҪ��
********************************************************************************/
#ifndef SHT10_H
#define SHT10_H
#include <ioCC2530.h> 
typedef signed   char   int8;
typedef unsigned char   uint8;

typedef signed   short  int16;
typedef unsigned short  uint16;

typedef signed   long   int32;
typedef unsigned long   uint32;


void IIC_GPIOConfig(void);
void SDA_OUT(void);
void SDA_IN(void);

static char WriteByte(unsigned char value);
static char ReadByte(unsigned char ack);
void ConnectionReset(void);
static void TransStart(void);

void SCLtest(void);
void SDAtest(void);
void GH_ReadSoilTemp_Humi(float *pTemp,float *pHumi);

void SHTXX_Init(void);
unsigned char Measure(unsigned char *p_value, unsigned char *p_checkum, unsigned char mode);
void SHTXX_Cal(unsigned short *temp, unsigned short *humi, float *f_temp, float *f_humi );

#endif