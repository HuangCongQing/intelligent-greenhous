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

void CO2_init(void);
float GetADC(void);

#endif