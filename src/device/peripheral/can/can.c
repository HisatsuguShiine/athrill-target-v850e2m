#include "inc/can.h"
#include "device.h"
#include "std_errno.h"
#include "mpu_types.h"
#include "cpuemu_ops.h"
#include "athrill_mros_device.h"
#include <stdio.h>

static Std_ReturnType can_get_data8(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 *data);
static Std_ReturnType can_get_data16(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint16 *data);
static Std_ReturnType can_get_data32(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint32 *data);
static Std_ReturnType can_put_data8(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 data);
static Std_ReturnType can_put_data16(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint16 data);
static Std_ReturnType can_put_data32(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint32 data);
static Std_ReturnType can_get_pointer(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 **data);

MpuAddressRegionOperationType	can_memory_operation = {
		.get_data8 		= 	can_get_data8,
		.get_data16		=	can_get_data16,
		.get_data32		=	can_get_data32,

		.put_data8 		= 	can_put_data8,
		.put_data16		=	can_put_data16,
		.put_data32		=	can_put_data32,

		.get_pointer	= can_get_pointer,
};

#define MROS_PUB_REQ_NUM	2
#define MROS_SUB_REQ_NUM	2
static AthrillMrosDevPubReqType pub_req_table[MROS_PUB_REQ_NUM] = {
		{ .topic_name = "CANID_0x100", .pub = NULL },
		{ .topic_name = "CANID_0x101", .pub = NULL },
};
void topic_callback_01(const char *data, int datalen);
void topic_callback_02(const char *data, int datalen);
static AthrillMrosDevSubReqType sub_req_table[MROS_SUB_REQ_NUM] = {
		{ .topic_name = "CANID_0x400", .sub = NULL, .callback = topic_callback_01, .sub = NULL },
		{ .topic_name = "CANID_0x404", .sub = NULL, .callback = topic_callback_02, .sub = NULL },
};

static MpuAddressRegionType *can_data_region;
void topic_callback_01(const char *data, int datalen)
{
	uint32 copylen = datalen;
	if (copylen > 8) {
		copylen = 8;
	}
	memcpy(&can_data_region->data[0], data, copylen);
}
void topic_callback_02(const char *data, int datalen)
{
	uint32 copylen = datalen;
	if (copylen > 8) {
		copylen = 8;
	}
	memcpy(&can_data_region->data[8], data, copylen);
}

void device_init_can(MpuAddressRegionType *region)
{
	int err;
	can_data_region = region;
	set_athrill_task();
	err = athrill_mros_device_pub_register(pub_req_table, MROS_PUB_REQ_NUM);
	if (err < 0) {
		printf("error: athrill_mros_device_pub_register()\n");
		return;
	}
	err = athrill_mros_device_sub_register(sub_req_table, MROS_SUB_REQ_NUM);
	if (err < 0) {
		printf("error: athrill_mros_device_sub_register()\n");
		return;
	}
	err = athrill_mros_device_start();
	if (err < 0) {
		printf("error: athrill_mros_device_start()\n");
		return;
	}
	return;
}


static Std_ReturnType can_get_data8(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 *data)
{
	uint32 off = (addr - region->start);
	*data = *((uint8*)(&region->data[off]));

	return STD_E_OK;
}
static Std_ReturnType can_get_data16(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint16 *data)
{
	uint32 off = (addr - region->start);
	*data = *((uint16*)(&region->data[off]));
	return STD_E_OK;
}
static Std_ReturnType can_get_data32(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint32 *data)
{
	uint32 off = (addr - region->start);
	*data = *((uint32*)(&region->data[off]));
	return STD_E_OK;
}
static Std_ReturnType can_put_data8(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 data)
{
	uint32 off = (addr - region->start);
	uint32 mbox;
	*((uint8*)(&region->data[off])) = data;

	if ((addr >= VCAN_TX_FLAG_BASE) && (addr < (VCAN_TX_FLAG_BASE + VCAN_TX_FLAG_SIZE))) {
		mbox = addr - VCAN_TX_FLAG_BASE;
		if (mbox <= 1) {
			ros_topic_publish(pub_req_table[mbox].pub, &region->data[VCAN_TX_DATA_0(mbox) - region->start], 8U);
		}
	}

	return STD_E_OK;
}
static Std_ReturnType can_put_data16(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint16 data)
{
	uint32 off = (addr - region->start);
	*((uint16*)(&region->data[off])) = data;
	return STD_E_OK;
}
static Std_ReturnType can_put_data32(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint32 data)
{
	uint32 off = (addr - region->start);
	*((uint32*)(&region->data[off])) = data;
	return STD_E_OK;
}
static Std_ReturnType can_get_pointer(MpuAddressRegionType *region, CoreIdType core_id, uint32 addr, uint8 **data)
{
	uint32 off = (addr - region->start);
	*data = &region->data[off];
	return STD_E_OK;
}