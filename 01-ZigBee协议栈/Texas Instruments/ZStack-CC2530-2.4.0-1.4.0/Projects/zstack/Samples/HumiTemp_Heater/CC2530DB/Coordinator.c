#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDProfile.h"
#include "ZDObject.h"
#include <string.h>
#include "Coordinator.h"
#include "DebugTrace.h"

#if !defined(WIN32)
#include "OnBoard.h"
#endif

#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
#include "OSAL_Nv.h"
#include "aps_groups.h"

#define SEND_TO_ALL_EVENT 0X01

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS]  = 
{
  GENERICAPP_CLUSTERID_1,GENERICAPP_CLUSTERID_2
};

const SimpleDescriptionFormat_t GenericApp_SimpleDesc = {
  GENERICAPP_ENDPOINT,                      //�˴��ں�
  GENERICAPP_PROFID,                        //Ӧ�ù淶ID
  GENERICAPP_DEVICEID,                      //Ӧ���豸ID
  GENERICAPP_DEVICE_VERSION,                //Ӧ���豸�汾��
  GENERICAPP_FLAGS,                          
  GENERICAPP_MAX_CLUSTERS,                   //����ذ������������
  (cId_t *)GenericApp_ClusterList,           //������б�
  0,                                         //����ذ������������
  (cId_t *)NULL                              //������б�
};

endPointDesc_t GenericApp_epDesc;
devStates_t GenericApp_NwkState;
byte GenericApp_TaskID;
byte GenericApp_TransID;
uint8 nodenum = 0;
uint16 addr[16];


void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pckt);
void GenericApp_SendTheMessage(void);
static void rxCB(uint8 port,uint8 event);
/*�������ܣ�Э����Э��ջ��ع��ܳ�ʼ��*/
void GenericApp_Init(byte task_id)
{
  halUARTCfg_t uartConfig;
  GenericApp_TaskID       = task_id;                     //��ʼ����������ȼ�
  GenericApp_TransID      = 0;                           //���������ݰ�����ų�ʼ��Ϊ0����ZigBeeЭ��ջ�У�ÿ����һ�����ݰ����÷�������Զ���1
  GenericApp_epDesc.endPoint = GENERICAPP_ENDPOINT;    
  GenericApp_epDesc.task_id = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq = noLatencyReqs;          //���������ǶԽڵ����������г�ʼ������ʽ��Ϊ�̶���һ�㲻��Ҫ�޸�
  afRegister(&GenericApp_epDesc);                          //ʹ��afRegister�������ڵ�����������ע�ᣬֻ��ע���Ժ󣬲ſ���ʹ��OSAL�ṩ��ϵͳ����
  
  uartConfig.configured     = TRUE;                      //ȷ���Դ��ڲ�����������
  uartConfig.baudRate       = HAL_UART_BR_115200;        //���ڲ�����Ϊ115200
  uartConfig.flowControl    = FALSE;                     //û��������
  uartConfig.callBackFunc   = rxCB;                      //���ڻص�����
  HalUARTOpen(0,&uartConfig);                            //����uartConfig�ṹ������еĳ�Ա����ʼ������0
  
  HalLedBlink(HAL_LED_2,0,50,250);                       //ʹLED��2��ʼ��˸
}
/*�������ܣ�Э�����������磬�������¼��������
*/
UINT16 GenericApp_ProcessEvent(byte task_id,UINT16 events)
{
 
  afIncomingMSGPacket_t *MSGpkt;                          //����һ��ָ�������Ϣ�ṹ���ָ��MSGpkt��
  if(events & SYS_EVENT_MSG)
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);  ////ʹ��osal_msg_receive��������Ϣ�����Ͻ�����Ϣ������Ϣ�а����˽��յ����������ݰ�
    while(MSGpkt)
    {
      switch(MSGpkt->hdr.event)
      {
       case ZDO_STATE_CHANGE:                           //�Խ��յ�����Ϣ���ͽ����жϡ�Э�����յ�ZDO_STATE_CHANGE��Ϣ���������ɹ�
          HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);         //Э���������ɹ���LED��2����         
        break;
      case AF_INCOMING_MSG_CMD:                         //��Э�����յ�AF_INCOMING_MSG_CMD��Ϣ������Э�����յ������ڵ㴫���������ݡ�
          GenericApp_MessageMSGCB(MSGpkt);              //���յ������ݽ��д���
        break;
      default:
        break;
      }
      osal_msg_deallocate((uint8 *)MSGpkt);             //���յ�����Ϣ������󣬾���Ҫ�ͷ���Ϣ��ռ�ݵĴ洢�ռ�
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);  //������һ����Ϣ���ٴ���Ϣ�����������Ϣ��Ȼ����������Ӧ�Ĵ���ֱ��������Ϣ��������Ϊֹ
    }
    return (events ^ SYS_EVENT_MSG);
  }
  return 0;
}
/*�������ܣ�Э�����յ����ն˵㷢���������ݲ��Ӵ���0��ӡ������
 Э�����������ն˽ڵ��ϴ�������̵�ַ
*/
void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pkt)
{
  char buf[128] = 0;                          //�������ݻ�����
  char i;
  switch(pkt->clusterId)
  {
  case GENERICAPP_CLUSTERID_1:              //���ݺ�ΪGENERICAPP_CLUSTERID_2�������û�����
    osal_memcpy(buf,pkt->cmd.Data,pkt->cmd.DataLength);  //���յ��Ĵ��ն˽ڵ㴫���������ݿ�����buf��
    HalUARTWrite(0,buf,pkt->cmd.DataLength);     //�Ӵ���0��ӡ�����յ������� 
    break;
  case GENERICAPP_CLUSTERID_2:                  //�ڵ������ַ��Ϣ�����ݺ�ΪGENERICAPP_CLUSTERID_1
    osal_memcpy(buf,pkt->cmd.Data,pkt->cmd.DataLength);
    i = buf[0];
    addr[i] = buf[1] *256 + buf[2];
    break;
  
  }
}
/*�������ܣ�����Э������ȡ�������ݲ������ݷ��ͳ�ȥ*/
static void rxCB(uint8 port,uint8 event)
{
    uint8 buf[128];
    uint8 len = 0,i=0;
    afAddrType_t my_DstAddr;               //����һ��afAddrType_t���͵Ľṹ�壬���ڶ��巢�ͽڵ�ķ��ͷ�ʽ����Ŀ��ڵ�ĵص�ַ
    len = HalUARTRead(0,buf,128);           //��ȡ�������ݣ��������ݴ洢��buf��������
    if(len != 0)
    {  
      osal_memcpy(&i,buf,1);
      my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;  //���巢�ͷ�ʽΪ������ʽ����Ӧ�Ļ��й㲥���鲥�ķ�ʽ
      my_DstAddr.endPoint = GENERICAPP_ENDPOINT;  //��ʼ���˿ں�
      my_DstAddr.addr.shortAddr = addr[i];        //Ŀ��ڵ�̵�ַ
      AF_DataRequest(&my_DstAddr,&GenericApp_epDesc,GENERICAPP_CLUSTERID_1,
                   len-1,buf+1,&GenericApp_TransID,AF_DISCV_ROUTE,AF_DEFAULT_RADIUS);

    }
}


