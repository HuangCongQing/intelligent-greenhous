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
#include "stdio.h"
#include "CO2.h"
#include "Fan.h"

#include "aps_groups.h"
#define  SEND_DATA_EVENT 0X01

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
  GENERICAPP_CLUSTERID_1,GENERICAPP_CLUSTERID_2
};
const SimpleDescriptionFormat_t GenericApp_SimpleDesc = 
{
  GENERICAPP_ENDPOINT,             //�˿ں�
  GENERICAPP_PROFID,               //Ӧ�ù淶ID
  GENERICAPP_DEVICEID,             //Ӧ���豸ID
  GENERICAPP_DEVICE_VERSION,       //Ӧ���豸�汾��
  GENERICAPP_FLAGS,
  0,                              //����ذ������������
  (cId_t *)NULL,                  //������б�
  GENERICAPP_MAX_CLUSTERS,        //����ذ������������
  (cId_t *)GenericApp_ClusterList //������б�
};

endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;
byte GenericApp_TransID;
devStates_t GenericApp_NwkState;

void Send_Value(void);
void MSG_Handler(afIncomingMSGPacket_t *pkt);


/*�������ܣ�Э��ջϵͳ���ܳ�ʼ��*/
void GenericApp_Init(byte task_id)
{
  halUARTCfg_t uartConfig;
  
  GenericApp_TaskID       = task_id;   //��ʼ����������ȼ�
  GenericApp_NwkState     = DEV_INIT; //����ýڵ�Ϊ�ն˽ڵ�
  GenericApp_TransID      = 0;       //���������ݰ�����ų�ʼ��Ϊ0����ZigBeeЭ��ջ�У�ÿ����һ�����ݰ����÷�������Զ���1
  GenericApp_epDesc.endPoint  = GENERICAPP_ENDPOINT;
  GenericApp_epDesc.task_id   = &GenericApp_TaskID;
  GenericApp_epDesc.simpleDesc = (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
  GenericApp_epDesc.latencyReq  = noLatencyReqs; //���������ǶԽڵ����������г�ʼ������ʽ��Ϊ�̶���һ�㲻��Ҫ�޸�
  afRegister(&GenericApp_epDesc);               //ʹ��afRegister�������ڵ�����������ע�ᣬֻ��ע���Ժ󣬲ſ���ʹ��OSAL�ṩ��ϵͳ����
  
  CO2_init();
  Fan_init();
  HalLedBlink(HAL_LED_2,0,50,250);   //ʹLED��2��ʼ��˸
}
/*�������ܣ��ն˽ڵ�״̬�����仯����������*/
UINT16 GenericApp_ProcessEvent(byte task_id,UINT16 events)
{
  afIncomingMSGPacket_t *MSGpkt; //����һ��ָ�������Ϣ�ṹ���ָ��MSGpkt��
  if(events & SYS_EVENT_MSG)
  {
    MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);  //ʹ��osal_msg_receive��������Ϣ�����Ͻ�����Ϣ������Ϣ�а����˽��յ����������ݰ�
    while(MSGpkt)
    {
      switch(MSGpkt->hdr.event)
      {
      case ZDO_STATE_CHANGE: //�Խ��յ�����Ϣ���ͽ����жϡ��ն˽ڵ��յ�ZDO_STATE_CHANGE��Ϣ�����ɹ���������
          osal_set_event(GenericApp_TaskID,SEND_DATA_EVENT); //����һ��SEND_DATA_EVENT����������һ����ʱ��
          HalLedSet(HAL_LED_2,HAL_LED_MODE_ON); //�ն˽ڵ���������LED��2����
        break;
        
      case AF_INCOMING_MSG_CMD:  //���ն˽ڵ��յ�AF_INCOMING_MSG_CMD��Ϣ�������ն˽ڵ���յ������ڵ㴫���������ݡ�
        MSG_Handler(MSGpkt);   //���յ������ݽ��д���
        break;
          
      default:
        break;
      }
      osal_msg_deallocate((uint8 *)MSGpkt); //���յ�����Ϣ������󣬾���Ҫ�ͷ���Ϣ��ռ�ݵĴ洢�ռ�
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);  //������һ����Ϣ���ٴ���Ϣ�����������Ϣ��Ȼ����������Ӧ�Ĵ���ֱ��������Ϣ��������Ϊֹ
    }
    return (events ^ SYS_EVENT_MSG);
  }
  if(events & SEND_DATA_EVENT)   //�ж��¼�Ϊ�û��Լ�������¼�
  {               
    Send_Value();
    osal_start_timerEx(GenericApp_TaskID,SEND_DATA_EVENT,5000);//����һ����ʱ������5000ms�󣬻��ٴβ���һ��SEND_DATA_EVENT�¼�
    return(events ^ SEND_DATA_EVENT);
  }
  return 0;
}


void MSG_Handler(afIncomingMSGPacket_t *pkt)
{
  char buf[30] = 0;      //�������ݻ�����
  switch(pkt->clusterId)
  {
  case GENERICAPP_CLUSTERID_1:  //���ݺ�ΪGENERICAPP_CLUSTERID_1�������û�����
    osal_memcpy(buf,pkt->cmd.Data,pkt->cmd.DataLength); //���յ��Ĵ�Э���������������ݿ�����buf��
    //HalUARTWrite(0,buf,pkt->cmd.DataLength); //�Ӵ���0��ӡ�����յ�������
    if(buf[0] == 0x01)    //����յ�������Ϊ0x01,���ˮ��
    {
      Open_Fan();
    }else if(buf[0] == 0x00)  //����յ�������Ϊ0x00,��ر�ˮ��
    {
      Close_Fan();
    }
    break;  
  }

}
void Send_Value(void)
{
  float CO2_value; 
  uint16 nwk;
  uint8 buf[128]={0},addr[3]={0};
  afAddrType_t my_DstAddr;      //����һ��afAddrType_t���͵Ľṹ�壬���ڶ��巢�ͽڵ�ķ��ͷ�ʽ����Ŀ��ڵ�ĵص�ַ
  CO2_value = GetADC();   //��ȡ������̼����������
  nwk = NLME_GetShortAddr();
  addr[0] = 0x02;
  addr[1] = (uint8)(nwk>>8);
  addr[2] = (uint8)(nwk & 0x00ff);
   
  sprintf(buf,"������̼Ũ�ȣ�%0.2f",CO2_value);
  my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit; //���巢�ͷ�ʽΪ������ʽ����Ӧ�Ļ��й㲥���鲥�ķ�ʽ
  my_DstAddr.endPoint = GENERICAPP_ENDPOINT;    //��ʼ���˿ں�
  my_DstAddr.addr.shortAddr = 0x0000;          //Ŀ��ڵ�̵�ַ��Э�����ض̵�ַʼ��Ϊ0x0000
  AF_DataRequest(&my_DstAddr,&GenericApp_epDesc,GENERICAPP_CLUSTERID_1,
                 strlen(buf),buf,&GenericApp_TransID,AF_DISCV_ROUTE,AF_DEFAULT_RADIUS);
  
  AF_DataRequest(&my_DstAddr,&GenericApp_epDesc,GENERICAPP_CLUSTERID_2,
                 3,addr,&GenericApp_TransID,AF_DISCV_ROUTE,AF_DEFAULT_RADIUS);
  
  
  /*AF_DataRequestԭ��Ϊ��
    afStatus_t AF_DataRequest( afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                           uint16 cID, uint16 len, uint8 *buf, uint8 *transID,
                           uint8 options, uint8 radius )
    stAddrΪ������Ŀ�ĵ�ַ���˵��ַ���˵�ţ��ʹ���ģʽ
    cID����Profileָ������Ч�ļ�Ⱥ�ţ��ɼ����Ϊ����ID��
    len:�������ݳ���
    buf :�������ݻ�����
    transID:����ID��
    */
}



                          