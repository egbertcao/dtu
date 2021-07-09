#include <stdio.h>
#include "global_types.h"
#include "oc_mqtt.h"
#include "osa.h"
#include "oc_uart.h"
#include "dtu_common.h"

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;

static void mqtt_recv_cb(  unsigned short packet_id, char *topic,  unsigned int payload_len, char *payload)
{
	payload[payload_len] = '\0';
	OC_UART_LOG_Printf("%s packet_id:%d,topic:%s,payload_len:%d,payload:%s\n", __func__,packet_id,topic,payload_len,payload);
}

static void mqtt_event_cb(  oc_mqtt_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\n", __func__,event_type);
}

void mqtt_worker_thread(void * argv)
{
	int bRet,regState,netStatus = 0;
	while(bRun)
	{
		OC_NW_GetRegState(&regState);   //查询驻网成功
		if(regState)
		{
			break;
		}
		OSATaskSleep(200);//sleep 1s
	}
	OC_NetOpen();   // 激活网络
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_GetNetStatus();  // 查询是否激活成功
		if(bRet)
		{
			break;//Open  network Success!
		}
		OSATaskSleep(100);
		
	}
	mqttconfig_t currentmqtt;
	if(get_mqtt_param(currentmqtt) < 0){
		OC_UART_LOG_Printf("[%s] Get Serial param failed.\n", __func__);
		return;
	}
	OC_UART_LOG_Printf("%s:Open  network Success!\n", __func__);
	OC_Mqtt_URCRegister(mqtt_recv_cb,mqtt_event_cb);
	OC_Mqtt_Config(currentmqtt.clientid,currentmqtt.username,currentmqtt.password);
	OC_Mqtt_Ipstart(currentmqtt.address, currentmqtt.port, currentmqtt.version);
	OC_Mqtt_Connect(1, 60);
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_Mqtt_State();
		OC_UART_LOG_Printf("%s: mqtt state = %d \n", __func__,bRet);
		if(bRet==2)
		{
			break;
		}
		OSATaskSleep(100);
	}
	OC_UART_LOG_Printf("%s:connect Success!\n", __func__);
	OC_Mqtt_Subscribe("test",0);
	OC_Mqtt_Subscribe("modbus_set",0);
	OC_Mqtt_Subscribe("modbus_get",0);

	// 发送设备基本信息
	deviceinfo_t deviceinfo;
    device_info_get(&deviceinfo);
	cJSON *telemetry = NULL;
	cJSON *attributes = cJSON_CreateObject();
	cJSON_AddItemToObject(attributes, "imei", cJSON_CreateString(deviceinfo.imei));
    cJSON_AddItemToObject(attributes, "imsi", cJSON_CreateString(deviceinfo.imsi));
    cJSON_AddItemToObject(attributes, "iccid", cJSON_CreateString(deviceinfo.iccid));
	cJSON_AddItemToObject(attributes, "cgmr", cJSON_CreateString(deviceinfo.cgmr));
	OC_Mqtt_Publish("v1/devices/me/attributes", 0, 0, cJSON_PrintUnformatted(attributes));
	free(attributes);

	while(bRun)
	{
		// 循环发送
		deviceinfo_t deviceinfo;
		device_info_get(&deviceinfo);
		cJSON *telemetry = cJSON_CreateObject();
		cJSON_AddItemToObject(telemetry, "latitude", cJSON_CreateString(deviceinfo.latitude));
		cJSON_AddItemToObject(telemetry, "longitude", cJSON_CreateString(deviceinfo.longitude));
		cJSON_AddItemToObject(telemetry, "csq", cJSON_CreateString(deviceinfo.csq));
		cJSON_AddItemToObject(telemetry, "creg", cJSON_CreateString(deviceinfo.creg));
		OC_Mqtt_Publish("v1/devices/me/telemetry", 0, 0, cJSON_PrintUnformatted(telemetry));
		OSATaskSleep(10000);
	}
	free(telemetry);
}

void customer_app_mqtt_demo(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	bRun = TRUE;
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096,80,(char*)"demoMqttTask",
	                 mqtt_worker_thread, NULL) != 0)
	{
		return;
	}
}


