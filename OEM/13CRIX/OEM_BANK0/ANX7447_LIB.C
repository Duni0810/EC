/******************************************************************************
Copyright (c) 2017, Analogix Semiconductor, Inc.

PKG Ver  : V1.1.1
Filename : anx7447_lib.c
Project  : ANX7447 EC UCSI
Created  : 17 Jul 2017
Devices  : ANX7447
Toolchain: C51

Description:
Define ANX7447 chip operation interface.

Revision History:
V0.1
1. Initial version

v0.6.0
1. Used 1ms hook and 50ms hook to check Async callback, event, GPIO interrupt.
2. Used GPIO Port H.6 as Liberty Type-C port1 interrupt pin, configured as ¨INPUT〃, active is low.
3. Used GPIO Port H.5 as Liberty Type-C port0 interrupt pin, configured as ¨INPUT〃, active is low.
4. UCSI notify OPM by calling interface ¨ucsi_ppm_notify_opm〃.
5. Once EC received event from OPM, EC should copy all related data to data structure ¨ppm2bios〃 which has been defined in ucsi.c.
6. ANX7441_LIB.C will call all i2c_XXX_XXX_reg to access chip register space.
7. All UCSI code takes ~15K memory size, XDATA takes around 600Bytes.

v0.6.1
1. Remove 50ms timer, add 10ms timer.
2. Minor change for UCSI infrastructure.
3. Resolve/Fix EC get unexpected voltage/current(0) while work as power sink.
4. Resolve/Fix self-charging issue.
5. Resolve/Fix EC will hang up if ANX7447 no OCM inside or ANX7447 damaged issue.
6. Add OCM version for each port for BIOS checking.
7. UCSI preliminary working, can properly receive PPM_RESET/NOTIFICATION_ENABLE/CONNECTOR_CAPABILITY/CAPABILITY.

v0.6.2
1. Fix dead battery mode, system cannot power on with 40W type-c charger attached.
2. PD Controller(ANX7447/ANX7411) will synchronize booting with EC (PD start to run after EC running).
3. Add new interface to Re-driver (MUX) chip control for DP and USB3.X

v0.6.3
1. Add power/data role swap in the initial stage if OPM set fixed power/data role
2. Fix error ack command didn't set "CommandCompletedIndicator" issue.
3. Fix error command "Datalength" not 0x10 issue.
4. Enlarge command timeout to 2.5s.

v0.6.4
1. Turn off DisplayPort Alt Mode support feature in sink identity & svid.
2. Add send source/sink capability interface.
3. Add connector partner type checking in data role swap callback.
4. Add connector power role checking in power role swap callback.
5. Fix HLK "Get alt mode command" test item.
6. Fix HLK "Connector status - Error" test item.
7. Fix HLK "Connector status - fail connector reset in dead battery condition".

v0.6.5
1. Add battery out checking in "ppm_connector_reset" routing for UCSI "UCSI Get Connector Status - fail connector reset in dead battery" test item.
2. Fix JIRA LBT-215, Intel ID:1504599338 : GLK9(ANX7447 inside)+TAIKSEN adapter(HDMI+USB+Type-C charger)+AOC I2267 HDMI monitor. Plug in the adapter, no video output on monitor.
3. Fix USB TYPE-C ECR.

v0.6.6
1. support not calibrate chip, support AC chip.

v0.6.7
1.Fix apple dongle with Type-C charger attached, GLK cannot get high voltage charging.
2.Set VSafe0V to 1.8V.

v0.7.0
1. FIX HLK test items.
2. Add more UCSI connector capability.
3. Add new send source capability interface.
4. Adjust SET_UOR/SET_PDR swap timing control.

v0.7.1
1. Fix HLK test item, enlarge SET_PDR waiting time to 1.5s.

v0.8.0
1.Turn off Debug accessory support flag.
2.Turn off Audio accessory support flag.
3.Fix PD cannot power off/on, no action while attach/detach charger/dongle.
4.Fix Mass interrupt issue.
5.Release v0.8.0

v0.8.1
1. Fix not correctly clear PD interrupt issue.
2. Fix power direction not correctly report issue.

v0.8.2
1. Checking UOM PDM flags while send source/sink caps, JIRA LBT-410.
2. Send DP caps(source prefer) to firmware at chip initial stage, JIRA LBT-411.

v0.8.3
1. Fix JIRA:LBT-302(some kind of dongle cannot output 4K/60Hz video), Add PIN_ASSIGMENT_E support.
2. Shrink SET_PDR waiting time to 750ms, JIRA LBT-412.

v0.8.4
1. Fix several USB Type-C PD 2.0 CTS failed items, JIRA LBT-413.
2. Fix initialization message(such as SVID, DP_CAPS) didn't send to OCM on time issue, JIRA LBT-414.

v1.0.1
Based on v0.8.4.

v1.0.2
Fix JIRA LBT-366, LBT-404, LBT-429

v1.0.3
Add VDM interface, LBT-437

v1.0.5
1. Fix JIRA LBT-429
2. Add PR_CONSUMER_GOT_POWER interrupt, JIRA LBT-539
3. Code clean up

v1.0.6
1. Fix JIRA LBT-501 PD 3.0 CTS

v1.0.7
1. Fix HLK item failed if the second Type-C port have no device attached.
2. Add sending get_source_caps to request RDO control

v1.0.8
1. Disable SET_UOM feature

v1.0.9
1. Change the "Data Length" field in "GET_ALTERNATE_MODES" status from 4 times to 6 times.

v1.0.a
1. Add HPD/PIN ASSIGNMENT/CC STATUS/DETACH event callback interface, JIRA LBT-540

v1.0.b
1. remove OCM sending hardware reset event report, keep received, JIRA LBT-541.

v1.0.c
1. Enable SET_UOM feature, fix HLK failed item, JIRA LBT-552

v1.1.1
1. Change the version v1.0.c to v1.1.1.
******************************************************************************/
#include <stdio.h>
#include <string.h>

#include <UCSI.H>
#include <ANX7447_LIB.H>

#define AUTO_RDO_ENABLE

#define PATCH_NOT_CALIBRATED_CHIP

#define _BIT0   0x01
#define _BIT1   0x02
#define _BIT2   0x04
#define _BIT3   0x08
#define _BIT4   0x10
#define _BIT5   0x20
#define _BIT6   0x40
#define _BIT7   0x80
/***************************************************************/
/*Register definition of device address 0x58*/
#define TCPC_ROLE_CONTROL        0x1A
#define DRP_CONTROL              _BIT6
#define RP_VALUE                 (_BIT5|_BIT4)
#define CC2_CONTROL              (_BIT3|_BIT2)
#define CC1_CONTROL              (_BIT1|_BIT0)
#define DRP_EN                   _BIT6 // DRP_CONTROL

#define TCPC_COMMAND             0x23

#define ANALOG_CTRL_6            0xA6
#define ADC_CTRL_2               0xC0

#define ANALOG_CTRL_0            0xA0
#define DFP_OR_UFP               _BIT6

#define INTR_ALERT_0             0xCB
#define INTR_RECEIVED_MSG        _BIT7
#define INTR_SOFTWARE_INT        _BIT6

#define INTR_ALERT_1             0xCC
#define INTR_INTP_POW_ON         _BIT7 // digital powerup indicator
#define INTR_INTP_POW_OFF        _BIT6 


#define TCPC_CTRL_2              0xCD
#define SOFT_INTP_1              _BIT1

#define  TCPC_CONTROL            0x19

#define  POWER_CONTROL           0x1C

#define TX_OBJ1_BYTE_0           0x54

#define VBUS_VOLTAGE_0           0x70
#define VBUS_VOLTAGE_BIT7_0      0xFF

#define VBUS_VOLTAGE_1           0x71
#define VBUS_VOLTAGE_BIT9_8      (_BIT0 | _BIT1)

#define  PD_1US_PERIOD           0x80
#define  PD_TX_BIT_PERIOD        0x86

#define ANALOG_CTRL_1            0xA1
#define R_TOGGLE_ENABLE          _BIT7
#define R_LATCH_TOGGLE_ENABLE    _BIT6
#define TOGGLE_CTRL_MODE         _BIT5
#define CC2_VRD_USB              _BIT2
#define CC2_VRD_1P5              _BIT1
#define CC2_VRD_3P0              _BIT0

#define  ANALOG_CTRL_9           0xA9

#define ANALOG_CTRL_10           0xAA
#define FRSWAP_CTRL              0xAB
#define FR_SWAP                  _BIT7
#define FR_SWAP_EN               _BIT6
#define R_FRSWAP_CONTROL_SELECT  _BIT3
#define R_SIGNAL_FRSWAP          _BIT2
#define TRANSMIT_FRSWAP_SIGNAL   _BIT1
#define FRSWAP_DETECT_ENABLE     _BIT0

#define RING_OSC_CTRL            0xD9
#define VBUS_OCP_0               0xE6 
#define VBUS_OCP_1               0xE7
#define VBUS_OCP_BIT9_8          (_BIT0 | _BIT1)

/***************************************************************/
/*Register definition of device address 0x7a*/
#define TX_DATA_BYTE_30          0x00
 
/***************************************************************/
/*Register definition of device address 0x7e*/
#define R_RAM_LEN_H              0x03
#define FLASH_ADDR_EXTEND        _BIT7
 
#define R_RAM_CTRL               0x05
#define FLASH_DONE               _BIT7
#define BOOT_LOAD_DONE           _BIT6
#define LOAD_CRC_OK              _BIT5 // CRC_OK
#define LOAD_DONE                _BIT4

#define R_FLASH_ADDR_H           0x0c
#define R_FLASH_ADDR_L           0x0d

#define FLASH_WRITE_DATA_0       0xe
#define FLASH_READ_DATA_0        0x3c

#define R_FLASH_LEN_H            0x2e
#define R_FLASH_LEN_L            0x2f

#define R_FLASH_RW_CTRL          0x30
#define GENERAL_INSTRUCTION_EN   _BIT6
#define FLASH_ERASE_EN           _BIT5
#define WRITE_STATUS_EN          _BIT2
#define FLASH_READ               _BIT1
#define FLASH_WRITE              _BIT0

#define R_FLASH_STATUS_0         0x31

#define  FLASH_INSTRUCTION_TYPE  0x33
#define FLASH_ERASE_TYPE         0x34

#define R_FLASH_STATUS_REGISTER_READ_0  0x35
#define WIP                      _BIT0

#define R_I2C_0                  0x5C
#define read_Status_en           _BIT7

#define  OCM_CTRL_0              0x6e
#define OCM_RESET                _BIT6

#define ADDR_GPIO_CTRL_0         0x88
#define SPI_WP                   _BIT7
#define SPI_CLK_ENABLE           _BIT6
/*
* For SKIP highest voltage
* Maximum Voltage for Request Data Object
* 100mv units
*/
#define MAX_VOLTAGE 0xAC // 0x7E:0xAC
/*
* For selection PDO
* Maximum Power for Request Data Object
* 500mW units
*/
#define MAX_POWER 0xAD // 0x7E:0xAD
/*
* For mismatch
* Minimum Power for Request Data Object
* 500mW units
*/
#define MIN_POWER 0xAE // 0x7E:0xAE
/*Show Maximum voltage of RDO*/
#define RDO_MAX_VOLTAGE 0xAF // 0x7E:0xAF
/*Show Maximum Powe of RDO*/
#define RDO_MAX_POWER 0xB0 // 0x7E:0xB0
/*Show Maximum current of RDO*/
#define RDO_MAX_CURRENT 0xB1 // 0x7E:0xB1

#define FIRMWARE_CTRL 0xB2 // 0x7E:0xB2
#define disable_usb30 _BIT0
#define auto_pd_en _BIT1
#define trysrc_en _BIT2
#define trysnk_en _BIT3
#define support_goto_min_power _BIT4
#define snk_remove_refer_cc _BIT5
#define force_send_rdo _BIT6
#define high_voltage_for_same_power _BIT7

#define FW_STATE_MACHINE 0xB3 // 0x7E:0xB3

#define OCM_VERSION_REG 0xB4

#define INT_MASK 0xB6 // 0x7E:0xB6
/*same with 0x28 interrupt mask*/
#define CHANGE_INT 0xB7 // 0x7E:0xB7
#define RESV0 _BIT0
#define RECEIVED_ACK _BIT1
#define VCONN_CHANGE _BIT2
#define VBUS_CHANGE _BIT3
#define CC_STATUS_CHANGE _BIT4
#define DATA_ROLE_CHANGE _BIT5
#define PR_CONSUMER_GOT_POWER _BIT6
#define HPD_STATUS_CHANGE _BIT7

#define SYSTEM_STSTUS 0xB8 // 0x7E:0xB8
/*0: VCONN off; 1: VCONN on*/
#define VCONN_STATUS _BIT2
/*0: vbus off; 1: vbus on*/
#define VBUS_STATUS _BIT3
/*1: host; 0:device*/
#define S_DATA_ROLE _BIT5
/*0: Chunking; 1: Unchunked*/
#define SUPPORT_UNCHUNKING _BIT6
/*0: HPD low; 1: HPD high*/
#define HPD_STATUS _BIT7

#define NEW_CC_STATUS 0xB9 // 0x7E:0xB9

// PD Revision configure
// 0: default, 1:PD_REV20, 2:PD_REV30
#define PD_REV_INIT 0xBA // 0x7E:0xBA

#define PD_EXT_MSG_CTRL 0xBB // 0x7E:0xBB
#define SRC_CAP_EXT_REPLY _BIT0
#define MANUFACTURER_INFO_REPLY _BIT1
#define BATTERY_STS_REPLY _BIT2
#define BATTERY_CAP_REPLY _BIT3
#define ALERT_REPLY _BIT4
#define STATUS_REPLY _BIT5
#define PPS_STATUS_REPLY _BIT6
#define SNK_CAP_EXT_REPLY _BIT7


#define PD_MAX_VOLTAGE 0xc8 /*20V, 100mv per unit*/ 	
#define PD_MAX_POWER   0x82 /*65W, 500mW per unit*/
#define PD_MIN_POWER   0x02 /* 1W */


// Battery Status
struct battery_status_data_obj
{
    u8 reserved; // Reserved
    u8 battery_info; // Battery Info
    u16 battery_pc; // Battery PC
};

// Alert Message
struct alert_data_obj
{
    u16 reserved     :16; // Reserved
    u8 hot_swappable_batteries :4; // Battery Info
    u8 fixed_batteries :4; // Battery Info
    u8 type_of_alert   :8; // Battery PC
};

// Source Capabilities Extended
struct source_cap_extended
{
    u16 VID;
    u16 PID;
    u32 XID;
    u8 fw_version;
    u8 hw_version;
    u8 voltage_regulation;
    u8 holdup_time;
    u8 compliance;
    u8 touch_current;
    u16 peak_current1;
    u16 peak_current2;
    u16 peak_current3;
    u8 touch_temp;
    u8 source_inputs;
    u8 batteries;
    u8 source_PDP;
};

// Sink Capabilities Extended
struct sink_cap_extended
{
    u16 VID;
    u16 PID;
    u32 XID;
    u8 fw_version;
    u8 hw_version;
    u8 Reserved[11];
    u8 sink_inputs;
    u8 batteries;
    u8 sink_PDP;
};


// Status Message
struct status_data
{
    u8 internal_temp;
    u8 present_input;
    u8 present_battery_input;
    u8 event_flags;
    u8 temperature_status;
};

// Battery Cap
struct battery_cap
{
    u16 VID;
    u16 PID;
    u16 battery_design_cap;
    u16 battery_last_full_charge_cap;
    u8 battery_type;
};

// Manufacturer Info
struct manufacturer_info
{
    u8 manufacturer_info_target;
    u8 manufacturer_info_ref;
};

// Manufacturer Info Data
struct manufacturer_info_data
{
    u16 VID;
    u16 PID;
    u8 manufacturer_string[22];
};

// PPS Status Message
struct pps_status_data
{
    u16 output_voltage;
    u8 output_current;
    u8 real_time_flags;
};

struct ext_message_header
{
    u16 data_size                  :9; // Data Size
    u16 reserved                   :1; // Reserved
    u16 request_chunk              :1; // Request Chunk
    u16 chunk_number               :4; // Chunk Number
    u16 chunked                    :1; // Chunked
};

#define BYTE_SWAP(word) do { (word) = ((word)<<8)|((word)>>8); } while(0)
//low byte + high byte
#define USB_PD_EXT_HEADER(sendbuf, size, request, number, chunk) \
    do { \
        ((struct ext_message_header *)sendbuf)->data_size = size; \
        ((struct ext_message_header *)sendbuf)->reserved = 0; \
        ((struct ext_message_header *)sendbuf)->request_chunk = request; \
        ((struct ext_message_header *)sendbuf)->chunk_number = number; \
        ((struct ext_message_header *)sendbuf)->chunked = chunk; \
     } while(0)


static u8 interface_send_ctr_msg();
static u8 interface_send_msg();
static u8 send_pd_msg(PD_MSG_TYPE type,
        u8 *buf, u8 size, u8 type_sop);


#define INTERFACE_SEND_BUF_SIZE 32
#define INTERFACE_RECV_BUF_SIZE 32

extern u8 InterfaceSendBuf[PD_MAX_INSTANCE][INTERFACE_SEND_BUF_SIZE];
extern u8 InterfaceRecvBuf[PD_MAX_INSTANCE][INTERFACE_RECV_BUF_SIZE];
extern struct anx_context ANXXDATA context[PD_MAX_INSTANCE];
extern u8 ANXXDATA i;
extern u8 ANXXDATA j;
extern u8 ANXXDATA k;
extern u8 ANXXDATA l;
extern struct ppm_ucsi_parameters ANXXDATA pup;
extern u8 ANXXDATA role;
extern u8 ANXXDATA c;
extern u8 ANXXDATA len;
extern u16 ANXXDATA count;
extern u8 ANXXDATA port_id;
extern u8 ANXXDATA change_int;
extern u8 ANXXDATA change_status;
extern u8 ANXXDATA intr_alert_0;
extern u8 ANXXDATA intr_alert_1;
extern u16 ANXXDATA pdo_h;
extern u16 ANXXDATA pdo_l;
extern u32 ANXXDATA pdo_max;
extern u32 ANXXDATA pdo_max_tmp;
extern u8 ANXXDATA supply_1500ma_flag;
extern u8 ANXXDATA supply_1500ma_port;
extern u8 ANXXDATA random_data;
extern u8 ANXXDATA timer1msCount;
extern u8 ANXXDATA self_charging_error;
extern u8 ANXXDATA request_src_caps_flag[];

/* Fix JIRA LBT-501 */
#define VENDOR_ID  0x1F29
#define PRODUCT_ID 0x7447
#define HW_REV 0x01
#define SI_REV 0x04
#define FW_MAJOR_REV 0x02
#define FW_MINOR_REV 0x01


#define BYTE_CONST_SWAP(word) ((unsigned short int)(((word)<<8)|((word)>>8)))
/***Define Global Variables***/
// Battery Cap - Invalid Battery
code struct battery_cap  pd_battery_cap_invalid = {
    BYTE_CONST_SWAP(VENDOR_ID), BYTE_CONST_SWAP(PRODUCT_ID), 0xFFFF, 0xFFFF, 1
};

// Battery Cap
code struct battery_cap   pd_battery_cap[8] = {
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 0 
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 1 // Invalid Battery reference, Battery's cap unknown
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 2 // Hot Swappable Battery, Battery's cap unknown
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 3
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 4
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 5
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 6
	{ VENDOR_ID, PRODUCT_ID, 0xFFFF, 0xFFFF, 1 }, // Battery 7
};

//struct battery_cap recv_battery_cap;

// Battery Status - Invalid Battery
code struct battery_status_data_obj pd_battery_status_invalid = {
    0, 1, 0xFFFF // Reserved, Invalid Battery reference, Battery'sSOC unknown
};
// Battery Status
code struct battery_status_data_obj  pd_battery_status[8] = {
	{ 0, 0x01, 0xFFFF }, // Battery 0 
	{ 0, 0x01, 0xFFFF }, // Battery 1 // Reserved, Invalid Battery reference, Battery'sSOC unknown
	{ 0, 0x01, 0xFFFF }, // Battery 2 // Reserved, Battery charging, Battery'sSOC unknown
	{ 0, 0x1, 0xFFFF }, // Battery 3
	{ 0, 0x1, 0xFFFF }, // Battery 4
	{ 0, 0x1, 0xFFFF }, // Battery 5
	{ 0, 0x1, 0xFFFF }, // Battery 6
	{ 0, 0x1, 0xFFFF }, // Battery 7
};

//struct battery_status_data_obj  recv_battery_status;

// Alert Message
code struct alert_data_obj  pd_alert_data_obj = {
	0, // Reserved
	0, // Hot Swappable Battery 0~3
	1, // Fixed Battery 0~3
	2  // Battery Status Change
};
//struct alert_data_obj  recv_alert_data_obj;

// Get Battery Status
//unsigned char  pd_get_battery_status_ref = 4; // hot swap battery 0

// Source Capabilities Extended
code struct source_cap_extended  pd_source_cap_extended = {
	BYTE_CONST_SWAP(VENDOR_ID), // VID
	BYTE_CONST_SWAP(PRODUCT_ID), // PID
	0, // XID
	FW_MAJOR_REV, // FW Ver
	HW_REV, // HW Ver
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0x0, //battery number
	0x7  //PDP (4.5W)
};
//struct source_cap_extended  recv_source_cap_extended;

// Sink Capabilities Extended
code struct sink_cap_extended  pd_sink_cap_extended = {
	VENDOR_ID, // VID
	PRODUCT_ID, // PID
	0, // XID
	FW_MAJOR_REV, // FW Ver
	HW_REV, // HW Ver
	// ....
};
//struct sink_cap_extended  recv_sink_cap_extended;

// Status Message
code struct status_data  pd_status_data = {
    0, // not supported
    2, // External DC power
    0, // no battery
    0, // Event Flags
    0  // not Supported
};
//struct status_data  recv_status_data;

// Get Battery Cap
//unsigned char  pd_get_battery_cap_ref = 4; // hot swap battery 0

// Manufacturer Info
code struct manufacturer_info  pd_manufacturer_info = {
	0, // Port/Cable
	0  // Not battery
};

// Manufacturer Info Data
struct manufacturer_info_data  pd_manufacturer_info_data = {
	VENDOR_ID, // VID
	PRODUCT_ID, // PID
	"Analogix(MCU)" // String
};
//struct manufacturer_info_data  recv_manufacturer_info_data;
//Country code
code unsigned char  pd_country_code_data[3 * 2 + 2] = {
    0x03, 0x00, // Number of country codes in the message
    'A', 'U',
    'B', 'R',
    'C', 'A',
};
//Country info
code unsigned char  pd_country_code_info[8] = { //for test
   'A', 'U',
   0x00, 0x00,  //reserved
   0x00, 0x01, 
   0x02, 0x02, 
};
// PPS Status Message
code struct pps_status_data  pd_pps_status_data = {
	0xFFFF, // not supported
	0xFF, // not supported
	0, // PTF: 00 ?V Not Supported, Constant Voltage mode
};
//struct pps_status_data  recv_pps_status_data;

code u32 dfp_caps = PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_1500MA, PDO_FIXED_DATA_SWAP | PDO_FIXED_EXTERNAL);
code u32 src_caps = PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_1500MA, PDO_FIXED_COMM_CAP);
code u32 dfp_caps_default_ma = PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_1500MA, PDO_FIXED_DATA_SWAP | PDO_FIXED_EXTERNAL);
//code u32 src_caps_default_ma = PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_1500MA, PDO_FIXED_COMM_CAP);
code u32 src_caps_default_ma = PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_3A, PDO_FIXED_COMM_CAP);
code u8 set_variable_1[] = {IF_VAR_fw_var_reg, 0x00, 0x02, 0x01, 0x0d};

code u8 src_pdo[] = {0x5A, 0x90, 0x01, 0x2A, 0x96, 0x90, 0x01, 0x2A};
code u8 snk_identity[] = {
    //JIRA LBT-413
    #ifdef ANXDONGLE
    0x00, 0x00, 0x00, 0xec,/*snk_id_hdr */
    #else
    /* JIRA LBT-501 */
    0x00, 0x00, 0x00, 0x82,/*snk_id_hdr */
    #endif
	0x00, 0x00, 0x00, 0x00,	/*snk_cert */
	0x00, 0x00, 0x00, 0x00,	/*snk_prd*/
    //JIRA LBT-413
    #ifdef ANXDONGLE
	0x39, 0x00, 0x00, 0x51,	/*5snk_ama*/
    #endif
    };
code u8 svid[] = {0x00, 0x00, 0x00, 0xff };
code u8 enter_exit_svid[] = {0xff, 0x01, 0x01, 0x00};
#if 0
code u32 snk_cap[] = {/*5V, 0.9A, Fixed */PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_900MA, PDO_FIXED_FLAGS | PDO_FIXED_COMM_CAP),
             /*min 5V, max 20V, power 40W, battery */PDO_BATT(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_POWER_40W),
             /*min5V, max 20V, current 2A, variable */PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_CURRENT_2A)
};
#endif
code u32 snk_cap[] = {PDO_FIXED(PD_VOLTAGE_5V, PD_CURRENT_900MA, PDO_FIXED_COMM_CAP), /*5V, 0.9A, Fixed */
                      PDO_VAR(PD_VOLTAGE_5V, PD_MAX_VOLTAGE_20V, PD_CURRENT_3A) /*5V, 20V, 3A, variable */
};

//JIRA LBT-411
code u8 dp_source_dp_caps[4] = {0xC6, 0x00, 0x00, 0x00};

code u8 Liberty_i2c_address[PD_MAX_INSTANCE][4] = {
#if Two_PD_Chip
	{0x56, 0x6e, 0x78, 0x86},
    {0x58, 0x7e, 0x7a, 0x84}
    //{0x54, 0x64, 0x68, 0x6c},
	//{0x58, 0x7e, 0x7a, 0x84}
#else
    {0x58, 0x7e, 0x7a, 0x84}
	//{0x56, 0x6e, 0x78, 0x86}
#endif

#ifdef ITE_EC_EVB_PORT1_TEST
#message  "ANX IC Address on EVB configuration"
    {0x52, 0x62, 0x6a, 0x74}  /* port 1: I2C address selected as 1 1 */
#else
#message  "ANX IC Address on MRD configuration"
    //{0x54, 0x64, 0x68, 0x6c}  /* port 1: I2C address selected as 1 0 */
#endif

};

#define is_unchunked() (anx_read_reg(context[port_id].spi, SYSTEM_STSTUS) & SUPPORT_UNCHUNKING)
#define EXT_HEADER_CHUNKED      1
#define EXT_HEADER_UNCHUNKED    0

s8 *result_to_str(u8 result_type)
{
    return  (result_type == CMD_SUCCESS) ? "Accept" :
        (result_type == CMD_FAIL) ? "Fail" :
        (result_type == CMD_BUSY) ? "Busy" :
        (result_type == CMD_REJECT) ? "Reject":         
        "Unknown";
}

s8 *interface_to_str(u8 header_type)
{
#if 1
    s8 * str;
    switch (header_type) {
    case TYPE_PWR_SRC_CAP:
            str = "PWR_SRC_CAP";
            break;
    case TYPE_PWR_SNK_CAP:
            str = "PWR_SNK_CAP";
            break;
    case TYPE_PWR_OBJ_REQ:
            str = "PWR_OBJ_REQ";
            break;
    case TYPE_DP_SNK_IDENTITY:
            str = "DP_SNK_IDENTITY";
            break;
    case TYPE_SVID:
            str = "SVID";
            break;
    case TYPE_PSWAP_REQ:
            str = "PSWAP_REQ";
            break;
    case TYPE_DSWAP_REQ:
            str = "DSWAP_REQ";
            break;
    case TYPE_GOTO_MIN_REQ:
            str = "GOTO_MIN_REQ";
            break;
    case TYPE_DP_ALT_ENTER:
            str = "DPALT_ENTER";
            break;
    case TYPE_DP_ALT_EXIT:
            str = "DPALT_EXIT";
            break;
    case TYPE_GET_SRC_CAP:
            str = "GET_SRC_CAP";
            break;
    case TYPE_GET_RDO:
            str = "GET_RDO";
            break;
    case TYPE_GET_SNK_CAP:
            str = "GET_SNK_CAP";
            break;
    case TYPE_VCONN_SWAP_REQ:
            str = "VCONN_SWAP_REQ";
            break;
    case TYPE_GET_DP_SNK_CAP:
            str = "GET_DP_SINK_CAP";
            break;
    case TYPE_DP_SNK_CFG:
            str = "DP_SNK_CFG";
            break;
    case TYPE_SOFT_RST:
            str = "Software Reset";
            break;
    case TYPE_HARD_RST:
            str = "Hardware Reset";
            break;
    case TYPE_RESTART:
            str = "Restart";
            break;
    case TYPE_GET_PD_STATUS:
            str = "PD_STATUS_REQ";
            break;
    case TYPE_ACCEPT:
            str = "ACCEPT";
            break;
    case TYPE_REJECT:
            str = "REJECT";
            break;
    case TYPE_VDM:
            str = "VDM";
            break;
    case TYPE_RESPONSE_TO_REQ:
            str = "RESPONSE_TO_REQ";
            break;
    case TYPE_EXT_SRC_CAP_EXT:
            str = "PD3_SRC_CAP";
            break;
    case TYPE_EXT_STS:
            str = "PD3_STS";
            break;
    case TYPE_EXT_GET_BATT_CAP:
            str = "PD3_GET_BATT_CAP";
            break;
    case TYPE_EXT_GET_BATT_STS:
            str = "PD3_GET_BATT_STS";
            break;
    case TYPE_EXT_BATT_CAP:
            str = "PD3_BATT_CAP";
            break;
    case TYPE_EXT_GET_MFR_INFO:
            str = "PD3_GET_MFR_INFO";
            break;
    case TYPE_EXT_MFR_INFO:
            str = "PD3_MFR_INFO";
            break;
    case TYPE_EXT_PDFU_REQUEST:
            str = "PD3_PDFU_REQUEST";
            break;
    case TYPE_EXT_PDFU_RESPONSE:
            str = "PD3_PDFU_RESPONSE";
            break;
    case TYPE_BATT_STS:
            str = "PD3_BATT_STS";
            break;
    case TYPE_ALERT:
            str = "PD3_ALERT";
            break;
    case TYPE_NOT_SUPPORTED:
            str = "PD3_NOT_SUPPORTED";
            break;
    case TYPE_GET_SRC_CAP_EXT:
            str = "PD3_GET_SRC_CAP";
            break;
    case TYPE_GET_STS:
            str = "PD3_GET_SRC_STS";
            break;
    case TYPE_FR_SWAP:
            str = "PD3_FR_SWAP";
            break;
    case TYPE_GET_PPS_STS:
            str = "PD3_GET_PPS_STS";
            break;
    case TYPE_FR_SWAP_SIGNAL:
            str = "PD3_FR_SWAP_SIGNAL";
            break;
    case TYPE_GET_COUNTRY_INFO:
            str = "PD3_GET_COUNTRY_INFO";
            break;
    case TYPE_DP_DISCOVER_MODES_INFO:
            str = "TYPE_DP_DISCOVER_MODES_INFO";
            break;
    case TYPE_GET_DP_CONFIGURE:
            str = "TYPE_GET_DP_CONFIGURE";
            break;
    case TYPE_DP_CONFIGURE:
            str = "TYPE_DP_CONFIGURE";
            break;
    case TYPE_I2C_MASTER_WRITE:
            str = "TYPE_I2C_MASTER_WRITE";
            break;
    case TYPE_GET_COUNTRY_CODE:
            str = "PD3_GET_COUNTRY_CODE";
            break;
    case TYPE_GET_SINK_CAP_EXT:
            str = "PD3_GET_SINK_CAP_EXT";
            break;
    case TYPE_EXT_SINK_CAP_EXT:
            str = "PD3_EXT_SINK_CAP_EXT";
            break;
    case TYPE_EXT_PPS_STS:
            str = "PD3_EXT_PPS_CAP_STS";
            break;
    case TYPE_EXT_COUNTRY_INFO:
            str = "PD3_EXT_COUNTRY_INFO";
            break;
    case TYPE_EXT_COUNTRY_CODE:
            str = "PD3_EXT_COUNTRY_CODE";
            break;
    case TYPE_GET_VAR:
            str = "TYPE_GET_VAR";
            break;
    case TYPE_SET_VAR:
            str = "TYPE_SET_VAR";
            break;
    default:
            str = "Unknown";
            break;
    }
#endif
    return str;
}

u8 get_data_role()
{
    /*fetch the data role */
    k = anx_read_reg(context[port_id].spi, SYSTEM_STSTUS);

    return (k & S_DATA_ROLE) != 0;

}


u8 get_power_role()
{
    /*fetch the power role */
    k = anx_read_reg(context[port_id].spi, SYSTEM_STSTUS);

    return (k & VBUS_STATUS) != 0;
}

#define InterfaceSendBuf_Addr 0xC0
#define InterfaceRecvBuf_Addr 0xE0

#define recvd_msg_len()          ((context[port_id].InterfaceRecvBuf[0] & 0x1F) - 1)
#define recvd_msg_sop_type()     (context[port_id].InterfaceRecvBuf[0]  >> 6 )
#define recvd_msg_type()         (context[port_id].InterfaceRecvBuf[1])
#define recvd_msg_buf()          (&context[port_id].InterfaceRecvBuf[2])
#define send_msg_len()           (context[port_id].InterfaceSendBuf[0] & 0x1F)
#define RESPONSE_REQ_TYPE()      (context[port_id].InterfaceRecvBuf[2])
#define RESPONSE_REQ_RESULT()    (context[port_id].InterfaceRecvBuf[3])

/* define max request current 3A and voltage 5V */
#define MAX_REQUEST_VOLTAGE 20000
#define MAX_REQUEST_CURRENT 3000
#define set_rdo_value(v0, v1, v2, v3)   \
    do {                \
        context[port_id].pd_rdo[0] = (v0);  \
        context[port_id].pd_rdo[1] = (v1);  \
        context[port_id].pd_rdo[2] = (v2);  \
        context[port_id].pd_rdo[3] = (v3);  \
    } while (0)

#ifdef ANXREDRIVER
#define plug_orientation(port) (anx_read_reg(context[port].spi,NEW_CC_STATUS)& 0x0d)
void redriver_mux_control(unsigned char mode, unsigned char cc_direction)  //redriver control 
{
    unsigned char buf[3];
    buf[0] = REDRIVER_ADDRESS; //address
    buf[1] = REDRIVER_OFFSET; //offset
    if (cc_direction == CC1_CONNECTED) {
        if (mode == USB3_1_DP_2LANES) {
            buf[2] = 0xf8; //data
        } else if (mode == DP_ALT_4LANES) {
            buf[2] = 0xe8; //data
        }  else if (mode == USB3_1_CONNECTED) {
            buf[2] = 0xb8; //data
        }
    } else {
        if (mode == USB3_1_DP_2LANES) {
            buf[2] = 0xfc; //data
        } else if (mode == DP_ALT_4LANES) {
            buf[2] = 0xec; //data
		} else if (mode == USB3_1_CONNECTED) {
            buf[2] = 0xbc; //data
        }
    }
    send_pd_msg((PD_MSG_TYPE)TYPE_I2C_MASTER_WRITE, buf, 3, 0);
}
void dp_configure_redriver(unsigned char mode) 
{
    unsigned char mux_mode;
    //Fix JIRA LBT-302
    if((mode == SELECT_PIN_ASSIGMENT_C) || (mode == SELECT_PIN_ASSIGMENT_E) )
        mux_mode = DP_ALT_4LANES;
    else
        mux_mode = USB3_1_DP_2LANES;
    if(context[port_id].mux_mode != mux_mode) {
        redriver_mux_control(mux_mode, context[port_id].cc_orientation);
        context[port_id].mux_mode = mux_mode;
    }
}
#endif
/* default request max RDO */
void build_rdo_from_source_caps_new(u8 obj_cnt, u8 *buf)
{
	if(context[port_id].sel_voltage_pdo_index + 1 > obj_cnt)
	{
		for (i= 0; i < obj_cnt; i++){
		{
			((unsigned char *)&pdo_max_tmp)[0] = buf[4*i+3];
			((unsigned char *)&pdo_max_tmp)[1] = buf[4*i+2];
			((unsigned char *)&pdo_max_tmp)[2] = buf[4*i+1];
			((unsigned char *)&pdo_max_tmp)[3] = buf[4*i+0];
			switch(GET_PDO_TYPE(pdo_max_tmp))
			{	
				case (PDO_TYPE_FIXED >> 30):
					if((GET_PDO_FIXED_VOLT(pdo_max_tmp)) > pdo_max)
					{
						pdo_max = GET_PDO_FIXED_VOLT(pdo_max_tmp);
						context[port_id].sel_voltage_pdo_index = i;
					}
					break;
				case(PDO_TYPE_VARIABLE >> 30):
					if(GET_VAR_MAX_VOLT(pdo_max_tmp) > pdo_max)
					{
						pdo_max = GET_VAR_MAX_VOLT(pdo_max_tmp);
						context[port_id].sel_voltage_pdo_index = i;
					}
					break;
				case(PDO_TYPE_BATTERY >> 30):
					if(GET_BATT_MAX_VOLT(pdo_max_tmp) > pdo_max)
					{
						pdo_max = GET_BATT_MAX_VOLT(pdo_max_tmp);
						context[port_id].sel_voltage_pdo_index = i;
					}
					break;
				default:				
					break;
			}
			}
		}
	}
	else
	{
		((unsigned char *)&pdo_max_tmp)[0] = buf[context[port_id].sel_voltage_pdo_index*4+3];
		((unsigned char *)&pdo_max_tmp)[1] = buf[context[port_id].sel_voltage_pdo_index*4+2];
		((unsigned char *)&pdo_max_tmp)[2] = buf[context[port_id].sel_voltage_pdo_index*4+1];
		((unsigned char *)&pdo_max_tmp)[3] = buf[context[port_id].sel_voltage_pdo_index*4+0];
		switch(GET_PDO_TYPE(pdo_max_tmp))
		{
			case(PDO_TYPE_FIXED >> 30):
				pdo_max = GET_PDO_FIXED_VOLT(pdo_max_tmp);
				break;
			case(PDO_TYPE_VARIABLE >> 30):
				pdo_max = GET_VAR_MAX_VOLT(pdo_max_tmp);
				break;
			case(PDO_TYPE_BATTERY >> 30):
				pdo_max = GET_BATT_MAX_VOLT(pdo_max_tmp);
				break;
			default:
				break;
		}
	}
	((unsigned char *)&pdo_max_tmp)[0] = buf[context[port_id].sel_voltage_pdo_index*4+3];
	((unsigned char *)&pdo_max_tmp)[1] = buf[context[port_id].sel_voltage_pdo_index*4+2];
	((unsigned char *)&pdo_max_tmp)[2] = buf[context[port_id].sel_voltage_pdo_index*4+1];
	((unsigned char *)&pdo_max_tmp)[3] = buf[context[port_id].sel_voltage_pdo_index*4+0];
	switch(GET_PDO_TYPE(pdo_max_tmp))
	{
		case(PDO_TYPE_FIXED >> 30):
		case(PDO_TYPE_VARIABLE>>30):
			if(GET_PDO_FIXED_CURR(pdo_max_tmp) >= MAX_REQUEST_CURRENT)
			{
				context[port_id].ucsi_partner_rdo = RDO_FIXED(context[port_id].sel_voltage_pdo_index + 1, MAX_REQUEST_CURRENT, MAX_REQUEST_CURRENT, 0);
			}
			else
			{
				context[port_id].ucsi_partner_rdo = RDO_FIXED(context[port_id].sel_voltage_pdo_index + 1, GET_PDO_FIXED_CURR(pdo_max_tmp), GET_PDO_FIXED_CURR(pdo_max_tmp), RDO_CAP_MISMATCH);
			}
			break;
		case (PDO_TYPE_BATTERY >> 30):
			if((GET_BATT_OP_POWER(pdo_max_tmp)>>2) >= (MAX_REQUEST_CURRENT*(MAX_REQUEST_VOLTAGE/1000))>>2)
			{
				context[port_id].ucsi_partner_rdo = RDO_BATT(context[port_id].sel_voltage_pdo_index + 1, MAX_REQUEST_CURRENT*(MAX_REQUEST_VOLTAGE/1000), MAX_REQUEST_CURRENT*(MAX_REQUEST_VOLTAGE/1000), 0);
			}
			else
			{
				context[port_id].ucsi_partner_rdo = RDO_BATT(context[port_id].sel_voltage_pdo_index + 1, (unsigned long)(GET_BATT_OP_POWER(pdo_max_tmp)>>2)*4, (unsigned long)(GET_BATT_OP_POWER(pdo_max_tmp)>>2)*4, RDO_CAP_MISMATCH);
			}
			break;
		default:
			break;
	}
	
	
	//set_rdo_value(pd_rdo,sel_pdo & 0xff,(sel_pdo >> 8) & 0xff, (sel_pdo >> 16) & 0xff, (sel_pdo >> 24) & 0xff);
	
}

#ifndef AUTO_RDO_ENABLE
static u8 build_rdo_from_source_caps(u8 obj_cnt, u8 *buf)
{
    pdo_max = 0;
    obj_cnt &= 0x07;

    /* find the max voltage pdo */
    for (i = 0; i < obj_cnt; i++) {

        /* get max voltage now */
        pdo_max_tmp =
            (u16) ((((((((u16) buf[i * 4 + 3] << 8) | (buf[i * 4 + 2])) & 0xf) << 6) | ((((u16) buf[i * 4 + 1] << 8) | (buf[i * 4 + 0])) >> 10)) &
                    0x3ff) * 50);
        if (pdo_max_tmp > pdo_max) {
            pdo_max = pdo_max_tmp;
            pdo_l = (((u16) buf[i * 4 + 1] << 8) | (buf[i * 4 + 0]));
            pdo_h = (((u16) buf[i * 4 + 3] << 8) | (buf[i * 4 + 2]));
            context[port_id].sel_voltage_pdo_index = i;
        }
    }
    #ifdef ANXDEBUG
    ucsi_debug("maxV=%d, cnt %d index %d\n", pdo_max_tmp, obj_cnt,
            context[port_id].sel_voltage_pdo_index);
    #endif
    if ((pdo_h & (3 << 14)) != (PDO_TYPE_BATTERY >> 16)) {
        #ifdef ANXDEBUG
        ucsi_debug("maxMa %d\n", (u16) ((pdo_l & 0x3ff) * 10));
        #endif
        /* less than 900mA */
        if ((u16) ((pdo_l & 0x3ff) * 10) < MAX_REQUEST_CURRENT) {
            pdo_max =
                RDO_FIXED(context[port_id].sel_voltage_pdo_index + 1, (u16) ((pdo_l & 0x3ff) * 10),
                          (u16) ((pdo_l & 0x3ff) * 10), 0);
            pdo_max |= RDO_CAP_MISMATCH;
            set_rdo_value(pdo_max & 0xff, (pdo_max >> 8) & 0xff,
                          (pdo_max >> 16) & 0xff,
                          (pdo_max >> 24) & 0xff);
            return 1;
        } else {
            pdo_max =
                RDO_FIXED(context[port_id].sel_voltage_pdo_index + 1,
                          MAX_REQUEST_CURRENT, MAX_REQUEST_CURRENT,
                          0);
            set_rdo_value(pdo_max & 0xff, (pdo_max >> 8) & 0xff,
                          (pdo_max >> 16) & 0xff,
                          (pdo_max >> 24) & 0xff);

            return 1;
        }
    } else {
        pdo_max =
            RDO_FIXED(context[port_id].sel_voltage_pdo_index + 1, MAX_REQUEST_CURRENT,
                      MAX_REQUEST_CURRENT, 0);
        set_rdo_value(pdo_max & 0xff, (pdo_max >> 8) & 0xff,
                      (pdo_max >> 16) & 0xff, (pdo_max >> 24) & 0xff);
        return 1;
    }
    #ifdef ANXDEBUG
    ucsi_debug("RDO Mismatch !!!\n");
    #endif
    set_rdo_value(0x0A, 0x28, 0x00, 0x10);

    return 0;
}
#endif

static u32 change_bit_order(const u8 *pbuf)
{
    return ((u32)pbuf[3] << 24) | ((u32)pbuf[2] << 16) 
        | ((u32)pbuf[1] << 8) | pbuf[0];
}

static u8 pd_check_requested_voltage()
{
    if (!(context[port_id].ucsi_partner_rdo >> 28) || (context[port_id].ucsi_partner_rdo >> 28) > context[port_id].pd_src_pdo_cnt) {
#ifdef ANXDEBUG
        ucsi_debug("rdo = %x, Requested RDO is %d, Provided RDO number is %d\n", context[port_id].ucsi_partner_rdo, (u16)(context[port_id].ucsi_partner_rdo >> 28), (u8)context[port_id].pd_src_pdo_cnt);
#endif
        return 0; /* Invalid index */
    }
#ifdef ANXDEBUG
    ucsi_debug("pdo_max = %x\n", (change_bit_order(context[port_id].pd_src_pdo + (((context[port_id].ucsi_partner_rdo >> 28) - 1) * 4)) & 0x3ff));
#endif
    /* check current ... */
    if (((context[port_id].ucsi_partner_rdo >> 10) & 0x3FF) > (change_bit_order(context[port_id].pd_src_pdo + (((context[port_id].ucsi_partner_rdo >> 28) - 1) * 4)) & 0x3ff))//Update to pass TD.PD.SRC.E12 Reject Request
        return 0; /* too much op current */
    if ((context[port_id].ucsi_partner_rdo & 0x3FF) > (change_bit_order(context[port_id].pd_src_pdo + (((context[port_id].ucsi_partner_rdo >> 28) - 1) * 4)) & 0x3ff))//Update to pass TD.PD.SRC.E12 Reject Request
        return 0; /* too much max current */

    return 1;
}

void send_source_capability(void)
{
    #ifdef ANX_OHO_DFP_ONLY
    memcpy(context[port_id].pd_src_pdo, (u8 *)&dfp_caps, sizeof(dfp_caps));
	#else
    #ifdef ECR_CHECKING
    if ((supply_1500ma_port == port_id) && (supply_1500ma_flag == 1))
        memcpy(context[port_id].pd_src_pdo, (u8 *)&src_caps, 4);
    else
    #endif
        memcpy(context[port_id].pd_src_pdo, (u8 *)&src_caps_default_ma, 4);
    #endif
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_DWORD(&context[port_id].pd_src_pdo);
    #endif
    //Fix JIRA LBT-410
    if ((context[port_id].data_operation_mode_uor & OP_DRP) &&
		((context[port_id].data_operation_mode_uom & OP_DRP))){
        if ((context[port_id].power_operation_mode_pdr & PW_DRP) &&
			((context[port_id].power_operation_mode_pdm & PW_DRP)))
            context[port_id].pd_src_pdo[3] |=
                PDO_FIXED_HIGH_BYTE_DATA_SWAP | PDO_FIXED_HIGH_BYTE_DUAL_ROLE;
        else
            context[port_id].pd_src_pdo[3] |= PDO_FIXED_HIGH_BYTE_DATA_SWAP;
    } else {
        if ((context[port_id].power_operation_mode_pdr & PW_DRP) &&
			((context[port_id].power_operation_mode_pdm & PW_DRP)))
            context[port_id].pd_src_pdo[3] |= PDO_FIXED_HIGH_BYTE_DUAL_ROLE;
        else
            ;//default is no drp, no dual role
    }
    context[port_id].pd_src_pdo_cnt = 1;
    send_pd_msg(TYPE_CMD_SRC_CAP_1, (u8 *)&context[port_id].pd_src_pdo,
        4, SOP_TYPE);
    #ifdef ANXDEBUG
    ucsi_debug("Resend 1.5A source caps byte 3 is %.bx.\n", context[port_id].pd_src_pdo[3]);
    #endif
}
void send_sink_capability(void)
{
    memcpy(context[port_id].pd_snk_pdo, (u8 *)&snk_cap, 8);
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_DWORD(&context[port_id].pd_snk_pdo);
	SWAP_DWORD((u8 *)&context[port_id].pd_snk_pdo + 4);
    #endif
    //Fix JIRA LBT-410
    if ((context[port_id].data_operation_mode_uor & OP_DRP) &&
		((context[port_id].data_operation_mode_uom & OP_DRP))){
        if ((context[port_id].power_operation_mode_pdr & PW_DRP) &&
			((context[port_id].power_operation_mode_pdm & PW_DRP)))
            context[port_id].pd_snk_pdo[3] |=
                PDO_FIXED_HIGH_BYTE_DATA_SWAP | PDO_FIXED_HIGH_BYTE_DUAL_ROLE;
        else
            context[port_id].pd_snk_pdo[3] |= PDO_FIXED_HIGH_BYTE_DATA_SWAP;
    } else {
        if ((context[port_id].power_operation_mode_pdr & PW_DRP) &&
			((context[port_id].power_operation_mode_pdm & PW_DRP)))
            context[port_id].pd_snk_pdo[3] |= PDO_FIXED_HIGH_BYTE_DUAL_ROLE;
        else
            ;//default is no drp, no dual role
    }
    #ifdef ANXDEBUG
    ucsi_debug("Resend 1.5A sink caps, sink caps byte 3 is %.bx.\n",
        context[port_id].pd_snk_pdo[3]);
    #endif
    context[port_id].pd_snk_pdo_cnt = 2;
    send_pd_msg(TYPE_PWR_SNK_CAP, (u8 *)&context[port_id].pd_snk_pdo,
        8, SOP_TYPE);
}
void send_source_sink_capability(void)
{
    send_sink_capability();
    send_source_capability();
}
#ifdef ECR_CHECKING
static void Send_mismatch_source_caps(void)
{
    #ifdef ANXDEBUG
    ucsi_debug("Send mismatch source caps, 1.5A source caps.\n");
    #endif
    supply_1500ma_flag = 1;
    supply_1500ma_port = port_id;

    send_source_capability();
}
#endif

/*ChengLin: To integrate all recv_pd_xxxxxx_default_callback() functions to single one for reduce code size*/
static u8 Recv_PD_Commands_Default_Callback()
{
#ifdef ANXDEBUG
    u8 *pdo;
    u16 usb_vendor_id, usb_product_id;
#endif
    
    l = 1; // 0: Fail, 1: Pass 
    
    if (TYPE_HARD_RST != recvd_msg_type())
        context[port_id].ct = CABLE_TYPE_C;

    /*ChengLin: Special Check for Specific Commands*/
    if(recvd_msg_type() == TYPE_PWR_SRC_CAP)
    {
        if ( recvd_msg_len()  %4 != 0)
            return 0;
    }
    else if(recvd_msg_type() == TYPE_PWR_SNK_CAP)
    {
    }
    else if(recvd_msg_type() == TYPE_PWR_OBJ_REQ)
    {
        if (recvd_msg_len()  != 4)
            return 1;
    }

    switch(recvd_msg_type())
    {
        case TYPE_PWR_SRC_CAP: //0x00
            #ifndef AUTO_RDO_ENABLE
            build_rdo_from_source_caps_new(recvd_msg_len() /4, recvd_msg_buf());
            set_rdo_value(context[port_id].ucsi_partner_rdo & 0xff,
                      ((context[port_id].ucsi_partner_rdo >> 8) & 0xff),
                      ((context[port_id].ucsi_partner_rdo >> 16) & 0xff),
                      ((context[port_id].ucsi_partner_rdo >> 24) & 0xff))
            send_pd_msg(TYPE_PWR_OBJ_REQ, &context[port_id].pd_rdo, 4, SOP_TYPE);
            #endif
            build_rdo_from_source_caps_new(recvd_msg_len() /4, recvd_msg_buf());
            send_pd_msg(TYPE_GET_RDO, 0, 0, SOP_TYPE);
            context[port_id].partner_pdo_length = recvd_msg_len() ;
            context[port_id].partner_pdo_sink_or_source = 1; //partner as source
            context[port_id].anx_power_role = 0;
            /*voltage->0x7e:0xaf, current:0x7e:0xb1*/
            context[port_id].anx_current_in_50ma = anx_read_reg(context[port_id].spi, 0xb1);
            context[port_id].anx_voltage_in_100mv = anx_read_reg(context[port_id].spi, 0xaf);
            #ifdef ReportNegotiatedPowerLevel
            context[port_id].csc.csc.NegotiatedPowerLevel = 1;
            #endif
            #ifdef ReportSupportedProviderCap
            context[port_id].csc.csc.SupportedProviderCap = 1;
            #endif
            #ifdef ReportExternalSupply
            context[port_id].csc.csc.ExternalSupply = 1;
            #endif
            context[port_id].BatteryCharging = BCS_NOMINAL_CHARGING;
            #ifdef ANXDEBUG
            ucsi_debug("current:%x, voltage:%x, rdo :%lx, rdo length(%.bx)\n", (u16)context[port_id].anx_current_in_50ma,
                (u16)context[port_id].anx_voltage_in_100mv, context[port_id].ucsi_partner_rdo, context[port_id].partner_pdo_length);
            #endif
            ucsi_async_notify_raise_up(100);
            memcpy(context[port_id].partner_pdo, recvd_msg_buf(), min(VDO_SIZE, recvd_msg_len() ));
            break;
        case TYPE_PWR_SNK_CAP://0x01
            //received peer's sink caps, just store it in SINK_PDO array
            context[port_id].partner_pdo_length = recvd_msg_len() ;
            context[port_id].partner_pdo_sink_or_source = 0;
            memcpy(context[port_id].partner_pdo, recvd_msg_buf(), min(VDO_SIZE, recvd_msg_len() ));
            if(recvd_msg_len()  > VDO_SIZE)
            {
                l = 0;
                break;
            }
            break;
        case TYPE_DP_SNK_IDENTITY://0x02
            #ifdef ANXDEBUG
            pdo = (u8 *)recvd_msg_buf();
            usb_vendor_id = (u16)pdo[1] << 8 | (u16)pdo[0];
            usb_product_id = (u16)pdo[11] << 8 | (u16)pdo[10];

            ucsi_debug("usb vendor id(0x%x), product id(0x%x).\n",
            usb_vendor_id, usb_product_id);
            #endif
            break;
        case TYPE_SVID://0x03
            break;
        case TYPE_ACCEPT://0x05
            break;
        case TYPE_REJECT://0x06
            break;
        case TYPE_PSWAP_REQ://0x10
            context[port_id].power_role_swap_flag = 1;
            context[port_id].anx_power_role = get_power_role();
            context[port_id].csc.csc.PowerOperationMode = 1;
            ucsi_async_notify_raise_up(50);
            break;
        case TYPE_DSWAP_REQ://0x11
            context[port_id].csc.csc.ConnectorPartner = 1;
            if (get_data_role() == 1)
                context[port_id].ucsi_connector_partner_type = CPT_UFP_ATTACHED;
			else
                context[port_id].ucsi_connector_partner_type = CPT_DFP_ATTACHED;
            ucsi_async_notify_raise_up(50);
            break;
        case TYPE_GOTO_MIN_REQ://0x12
            break;
        case TYPE_DP_SNK_CFG:
            break;
        case TYPE_DP_DISCOVER_MODES_INFO:
            context[port_id].peer_svid_length += 4;
            #ifdef ReportSupportedCAM
            context[port_id].csc.csc.SupportedCAM = 1;
            #endif
            context[port_id].ac[context[port_id].peer_svid_length/4 - 1].svid = 0xFF01;
            memcpy(&context[port_id].ac[context[port_id].peer_svid_length/4 - 1].mid, recvd_msg_buf(), 4);
            break;
        case TYPE_DP_ALT_ENTER://0x19
            send_pd_msg(TYPE_GET_DP_CONFIGURE, 0, 0, SOP_TYPE);
            context[port_id].ConnectorPartner |= CP_ALT_MODE;
            context[port_id].csc.csc.ConnectorPartner = 1;
            #ifdef ReportSupportedCAM
            context[port_id].csc.csc.SupportedCAM = 1;
            #endif
            ucsi_async_notify_raise_up(50);
            break;
        case TYPE_DP_ALT_EXIT://0x1A
            //TODO....
            break;
        case TYPE_PWR_OBJ_REQ:  //0x16
            //pdo = (u8 *)para;

            if (recvd_msg_len() != 4)
                break;
            context[port_id].ucsi_partner_rdo = recvd_msg_buf()[0] | ((u32)recvd_msg_buf()[1] << 8) | ((u32)recvd_msg_buf()[2] << 16) | ((u32)recvd_msg_buf()[3] << 24);
            #ifdef ECR_CHECKING
            if ((context[port_id].anx_vbus_status == 1) &&
                    (recvd_msg_buf()[3] & _BIT2) && supply_1500ma_flag == 0)
                Send_mismatch_source_caps();
            #ifdef ANXDEBUG
            ucsi_debug("RDO : %lx, vbus(%.bx), supply_flag(%.bx).\n",
                context[port_id].ucsi_partner_rdo, context[port_id].anx_vbus_status, supply_1500ma_flag);
            #endif
            #endif
            #ifdef ReportNegotiatedPowerLevel
            context[port_id].csc.csc.NegotiatedPowerLevel = 1;
            #endif
            #ifdef ReportSupportedProviderCap
            context[port_id].csc.csc.SupportedProviderCap = 1;
            #endif
            #ifdef AUTO_RDO_ENABLE
            //ucsi_async_notify_raise_up(50);
            break;
            #endif
            if (pd_check_requested_voltage()) {
                l = send_pd_msg(TYPE_ACCEPT, NULL, 0, SOP_TYPE);
            } else {
                l = send_pd_msg(TYPE_REJECT, NULL, 0, SOP_TYPE);
            }
            context[port_id].downstream_pd_cap = 1;
            break;
        case TYPE_GET_PPS_STS: //0x21
            break;
        case TYPE_BATT_STS://0x24
            break;
        case TYPE_ALERT: //0x25
            /* Fix JIRA LBT-501 */
            l = recvd_msg_buf()[2];
            
            if (recvd_msg_buf()[3] & 0x02) {
                if (l & 0x1) {
                    k = 0x04;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
                if (l & 0x2) {
                    k = 0x05;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
                if (l & 0x4) {
                    k = 0x06;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
                if (l & 0x8) {
                    k = 0x07;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
                if (l & 0x10) {
                    k = 0x00;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
                if (l & 0x20) {
                    k = 0x01;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
                if (l & 0x40) {
                    k = 0x02;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
                if (l & 0x80) {
                    k = 0x03;
                    send_pd_msg(TYPE_EXT_GET_BATT_STS,&k,1,SOP_TYPE);
                }
            } else {
                send_pd_msg(TYPE_GET_STS, NULL, 0, SOP_TYPE);
            }
            break;
        #ifdef ANXREDRIVER
        case TYPE_DP_CONFIGURE: //0x2a	redriver control		
            if ((context[port_id].pd_capability & PD_HAS_REDRIVER) &&
                  ((recvd_msg_buf()[0] != 0) || (recvd_msg_buf()[1] != 0) ||
                  (recvd_msg_buf()[2] != 0) || (recvd_msg_buf()[3] != 0)))
                dp_configure_redriver(recvd_msg_buf()[1]);
            /* LBT-540 */
            anx_ucsi_pin_assignment_event(port_id, recvd_msg_buf()[1]);
            break;
        #endif
        case TYPE_GET_SRC_CAP_EXT: //0x1d
            //Fix JIRA LBT-501
            send_pd_msg(TYPE_EXT_SRC_CAP_EXT, (unsigned char*)&pd_source_cap_extended, 24, SOP_TYPE);
            break;
        case TYPE_GET_STS: //0x1e
            send_pd_msg(TYPE_EXT_STS, (unsigned char*)&pd_status_data, 5, SOP_TYPE);
            break;
        case TYPE_EXT_SRC_CAP_EXT: //0x30
            break;
        case TYPE_GET_SINK_CAP_EXT:
            /* Fix JIRA LBT-501 */
            send_pd_msg(TYPE_EXT_SINK_CAP_EXT, (unsigned char*)&pd_sink_cap_extended, 24, SOP_TYPE);
            break;
        case TYPE_EXT_STS://0x31        
            break;
        case TYPE_EXT_GET_BATT_CAP://0x32
            /* Fix JIRA LBT-501 */
            if (recvd_msg_buf()[0] > 7)
                send_pd_msg(TYPE_EXT_BATT_CAP, (unsigned char*)&(pd_battery_cap_invalid), 9, SOP_TYPE);
            else
                send_pd_msg(TYPE_EXT_BATT_CAP, (unsigned char*)&pd_battery_cap, 9, SOP_TYPE);
            break;
        case TYPE_EXT_GET_BATT_STS://0x33
            if(recvd_msg_buf()[0] > 7)
                send_pd_msg(TYPE_BATT_STS, (unsigned char*)&(pd_battery_status_invalid), 4, SOP_TYPE);
            else
                send_pd_msg(TYPE_BATT_STS, (unsigned char*)&(pd_battery_status[recvd_msg_buf()[0]]), 4, SOP_TYPE);
            break;
        case TYPE_EXT_BATT_CAP://0x34
            break;
        case TYPE_EXT_GET_MFR_INFO://0x35
            /* Fix JIRA LBT-501 */
            //RamDebug(0xAA); RamDebug(recvd_msg_buf()[0]);
            if(recvd_msg_buf()[0] == 0)
                send_pd_msg(TYPE_EXT_MFR_INFO, (unsigned char*)&pd_manufacturer_info_data, 26, SOP_TYPE);
            else // 1:Battery, 2~255:Reserved
                send_pd_msg(TYPE_NOT_SUPPORTED, NULL, 0, SOP_TYPE);
            break;
        case TYPE_EXT_MFR_INFO: //0x36
            break;
        case TYPE_GET_COUNTRY_INFO: //0x38
            send_pd_msg(TYPE_EXT_COUNTRY_INFO, (unsigned char*)&pd_country_code_info, 8, SOP_TYPE);
            break;          
        case TYPE_GET_COUNTRY_CODE: //0x39
            send_pd_msg(TYPE_EXT_COUNTRY_CODE, (unsigned char*)&pd_country_code_data, 8, SOP_TYPE);
            break;
        case TYPE_EXT_COUNTRY_CODE:
            break;
        case TYPE_EXT_COUNTRY_INFO:
            break;
        case TYPE_EXT_PPS_STS://0x37        
            break;
        case TYPE_EXT_SINK_CAP_EXT: //0x3a          
            break;
        case TYPE_NOT_SUPPORTED: //0x1c
            break;
        case TYPE_SOFT_RST:
			RamDebug(0x33);
            #ifdef ReportPDResetComplete
			RamDebug(0x34);
            context[port_id].csc.csc.PDResetComplete = 1;
            ucsi_async_notify_raise_up(4);
            #endif
            break;
        case TYPE_HARD_RST:
			RamDebug(0x35);
            #ifdef ReportPDResetComplete
            #ifdef ANXDEBUG
                ucsi_debug("received hardware reset event.\n");
            #endif
            /* Fix LBT-541 */
            if ((recvd_msg_len() == 0) || recvd_msg_buf()[0]& _BIT1) {
                context[port_id].csc.csc.PDResetComplete = 1;
                ucsi_async_notify_raise_up(4);
				RamDebug(0x36);
                #ifdef ANXDEBUG
                ucsi_debug("notify hardware reset event to ucsi.\n");
                #endif
            }
            #endif
            break;
        case TYPE_FR_SWAP: //0x1f
        default:
            break;
    }

 return l;

}

/* Recieve response message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : should be null
  *   para_len : 0
  * return:  0, fail;   1, success
  */
static u8 Recv_PD_Cmd_Rsp_Default_Callback()
{
    #ifdef ANXDEBUG
    ucsi_debug("RESPONSE for %s is %s\n",
        interface_to_str(RESPONSE_REQ_TYPE()),result_to_str(RESPONSE_REQ_RESULT()));
    #endif

    switch(RESPONSE_REQ_TYPE()){
        case TYPE_DSWAP_REQ:
            //need_notice_pd_cmd  =1;
            //usb_pd_cmd_status = RESPONSE_REQ_RESULT();
            //dswap_response_got = 1;

            context[port_id].csc.csc.ConnectorPartner = 1;
            break;
            
        case TYPE_PSWAP_REQ:
            context[port_id].power_role_swap_flag = 1;

            break;
        case TYPE_VCONN_SWAP_REQ:
            //vswap_response_got = 1;
            //need_notice_pd_cmd = 1;           
            //usb_pd_cmd_status = RESPONSE_REQ_RESULT();
            break;
        case TYPE_GOTO_MIN_REQ:
            //gotomin_response_got = 1;
            //need_notice_pd_cmd = 1;           
            //usb_pd_cmd_status = RESPONSE_REQ_RESULT();
            break;
        case TYPE_PWR_OBJ_REQ:
            //rdo_response_got = 1;
            //need_notice_pd_cmd = 1;           
            //usb_pd_cmd_status = RESPONSE_REQ_RESULT();
            break;
        case TYPE_SOFT_RST:
            #ifdef ReportPDResetComplete
            context[port_id].csc.csc.PDResetComplete = 1;
            #endif
            break;
        case TYPE_HARD_RST:
            #ifdef ReportPDResetComplete
            context[port_id].csc.csc.PDResetComplete = 1;
            #endif
            break;
            
        default:
            break;
    } 

    return 1;
}

/* Recieve Power Delivery Unstructured VDM message's callback function.
  * it can be rewritten by customer just reimmplement this function,
  * through register_default_pd_message_callbacku_func  
  *  void *para : in this function it means PDO pointer
  *   para_len : means PDO length
  * return:  0, fail;   1, success
  */
static u8 Recv_PD_VDM_Defalut_Callback()
{
	if ((recvd_msg_sop_type() == SOP_TYPE) && (recvd_msg_type() == TYPE_VDM)) {
        if (memcmp(svid, recvd_msg_buf(), 4) == 0) {
            //Add customer self defined parse code here.
            //
        } else {
            /* Fix JIRA LBT-501 return unsupport msg */
            send_pd_msg(TYPE_NOT_SUPPORTED, NULL, 0, SOP_TYPE);
        }
        return 1;
    }
	
    if ((recvd_msg_sop_type() == SOP1_TYPE) && ((recvd_msg_buf()[0]&0x0F) == 0x1)) {
        if (recvd_msg_len() < 20) {
            #ifdef ANXDEBUG
            ucsi_debug("no cable vdo.\n");
            #endif
            return 1;
        }

        *(u32 *)&context[port_id].cv = ((u32)recvd_msg_buf()[19] << 24) | ((u32)recvd_msg_buf()[18] << 16) |
            ((u32)recvd_msg_buf()[17] << 8) | recvd_msg_buf()[16];
        return 1;
    }

    return 1;
}

static u8 Recv_Debug_Callback()
{
    switch(recvd_msg_type()) {
    case TYPE_GET_VAR:
        if(recvd_msg_buf()[0] == 0) // idata
        {
            #ifdef ANXDEBUG
            ucsi_debug("read 0x%x idata = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == 1) // xdata
        {
            #ifdef ANXDEBUG
            ucsi_debug("read 0x%x xdata = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_fw_var_reg) // REG_FW_VAR
        {
            #ifdef ANXDEBUG
            ucsi_debug("read REG_FW_VAR[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_pd_src_pdo)
        {
            #ifdef ANXDEBUG
            ucsi_debug("read pd_src_pdo[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_pd_snk_pdo)
        {
            #ifdef ANXDEBUG
            ucsi_debug("read pd_snk_pdo[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_pd_rdo_bak)
        {
            #ifdef ANXDEBUG
            ucsi_debug("read pd_rdo_bak[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_pd_rdo)
        {
            #ifdef ANXDEBUG
            ucsi_debug("read pd_rdo[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_DP_caps)
        {
            #ifdef ANXDEBUG
            ucsi_debug("read DP_cap[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_configure_DP_caps) // REG_FW_VAR
        {
            #ifdef ANXDEBUG
            ucsi_debug("read configure_DP_caps[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_src_dp_status) // REG_FW_VAR
        {
            #ifdef ANXDEBUG
            ucsi_debug("read src_dp_status[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_sink_svid_vdo) // REG_FW_VAR
        {
            #ifdef ANXDEBUG
            ucsi_debug("read sink_svid_vdo[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else if(recvd_msg_buf()[0] == IF_VAR_sink_identity) // REG_FW_VAR
        {
            #ifdef ANXDEBUG
            ucsi_debug("read sink_identity[0x%x] = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        else // ?data
        {
            #ifdef ANXDEBUG
            ucsi_debug("read 0x%x ?data = ",((u16)recvd_msg_buf()[1]<<8) + recvd_msg_buf()[2]);
            #endif
        }
        break;
    default:
        break;
    }

    return 1;
}

/*ChengLin: To integrate all send_xxx DATA Messages to one function for reduce code size*/
static u8 Send_PD_Data_Messages(u8 type_msg, u8 *DataBuff, u8 size)
{
    if ( NULL == DataBuff )
        return CMD_FAIL;

    if(type_msg == TYPE_DP_SNK_CFG)
    {
        memcpy(context[port_id].configure_DP_caps, DataBuff, 4);
        memcpy(context[port_id].InterfaceSendBuf + 2, DataBuff, 4); 
        memset(context[port_id].InterfaceSendBuf + 2 + 4, 0, 4);
        context[port_id].InterfaceSendBuf[0] = 4 + 4 + 1; // + cmd
    }
    else
    {
        if (type_msg == TYPE_DP_SNK_IDENTITY)
            memcpy(context[port_id].src_dp_caps, DataBuff, size);
        memcpy(context[port_id].InterfaceSendBuf + 2, DataBuff, size);
        context[port_id].InterfaceSendBuf[0] = size + 1; // + cmd
    }
    
    context[port_id].InterfaceSendBuf[1] = type_msg;

    return interface_send_msg();
}

/* Fix JIRA LBT-501 */
static u8 Send_PD_Extend_Messages(u8 type_msg, u8 *buff_ref, u8 size, u8 type_sop)
{
	if(type_msg == TYPE_EXT_PDFU_REQUEST || type_msg == TYPE_EXT_COUNTRY_INFO || type_msg == TYPE_EXT_COUNTRY_CODE)
	{
		if(size > 26)
		{
			context[port_id].InterfaceSendBuf[0] =(26 + 2 + 1) | (type_sop << 6); //	+ ext_header + cmd
			memcpy(context[port_id].InterfaceSendBuf + 4, buff_ref, 26);
			
			if(type_msg == TYPE_EXT_COUNTRY_INFO || type_msg == TYPE_EXT_COUNTRY_CODE)
                anx_write_block_reg(context[port_id].PD_EMTB_slave_id, 0 , size-26, (unsigned char *)(buff_ref+26));
		}
		else
		{
			context[port_id].InterfaceSendBuf[0] =(size + 2 + 1) | (type_sop << 6); //	+ ext_header + cmd
			memcpy(context[port_id].InterfaceSendBuf + 4, buff_ref, size);
		}

	}
	else
	{
		context[port_id].InterfaceSendBuf[0] = (size + 2 + 1) | (type_sop << 6); // + ext_header + cmd
		memcpy(context[port_id].InterfaceSendBuf + 4, buff_ref, size);		
	}
	
	context[port_id].InterfaceSendBuf[1] = type_msg;
	USB_PD_EXT_HEADER((context[port_id].InterfaceSendBuf + 2), size,0,0,0);

    SWAP_WORD((context[port_id].InterfaceSendBuf + 2));
	return interface_send_msg();
}


unsigned char Send_Interface_Messages(u8 type_msg, u8 *DataBuff, u8 size)
{
    memcpy(context[port_id].InterfaceSendBuf + 2, DataBuff, size);
    context[port_id].InterfaceSendBuf[0] = size + 1; // + cmd
    context[port_id].InterfaceSendBuf[1] = type_msg;
    return interface_send_msg();
}
/**
 * @desc:   The Interface AP set the VDM packet to Ohio 
 *
 * @param:  vdm:  object buffer pointer of VDM,
 *                              
 *                
 * 
 *                size: vdm packet size
 *                  
 * @return:  0: success 1: fail 
 *  
 */ 

/* Add VDM interface, LBT-437 */
 u8 send_vdm(u8 type_sop, u8 *vdm, unsigned char size)
{
	if ( NULL == vdm )
		return CMD_FAIL;
	if((size<4) || (size>28) || (size%4 != 0))
		return CMD_FAIL;

	context[port_id].InterfaceSendBuf[0] = (size + 1) | (type_sop << 6); // + cmd
    context[port_id].InterfaceSendBuf[1] = TYPE_VDM;
    memcpy(context[port_id].InterfaceSendBuf + 2, vdm, size);
	return interface_send_msg();//updated to fix OHO-423
}

/*ChengLin: To integrate all send_xxx_ control messages to one function for reduce code size*/
static u8 Send_PD_Control_Messages(u8 type_msg)
{
    context[port_id].InterfaceSendBuf[1] = type_msg;
    return interface_send_ctr_msg(); 
}

#define cac_checksum(buf, len) {\
    j = 0;\
    for (k = 0; k < len; k++)\
        j += *(buf + k);\
}


u8 send_pd_msg(PD_MSG_TYPE type,
        u8 *buf, u8 size, u8 type_sop)
{
    #ifdef ANXDEBUG
    ucsi_debug("SendMSG ->%s: \n", interface_to_str(type));
    #endif
    #if 1
    switch (type) {
        //Send Control Messages
        case TYPE_GET_DP_SNK_CAP: //send 4      
        case TYPE_ACCEPT://send 5   
        case TYPE_REJECT://send 6
        case TYPE_GET_SRC_CAP://send 9
        case TYPE_GET_PD_STATUS://send 17   
        case TYPE_GET_SNK_CAP://0x1B
        case TYPE_GET_SRC_CAP_EXT://send 1d
        case TYPE_GET_STS://send 1e
        case TYPE_FR_SWAP_SIGNAL://send 20  
        case TYPE_GET_PPS_STS: // send 21
        case TYPE_GET_COUNTRY_CODE://send 22
        case TYPE_GET_SINK_CAP_EXT://send 23
        case TYPE_NOT_SUPPORTED:
        case TYPE_SOFT_RST://send f1
        case TYPE_HARD_RST://send f2        
            Send_PD_Control_Messages(type);
            break;
        case TYPE_GET_RDO:
        case TYPE_GET_DP_CONFIGURE:
        case TYPE_PSWAP_REQ://send 10
        case TYPE_DSWAP_REQ://send 11
        case TYPE_GOTO_MIN_REQ://send 12
        case TYPE_VCONN_SWAP_REQ://send 13      
            if(Send_PD_Control_Messages(type) == CMD_SUCCESS)
                l = CMD_SUCCESS;
            else
                l = CMD_FAIL;
            break;

        //Send Data Messages
        case TYPE_PWR_SRC_CAP: //send 0
        case TYPE_PWR_SNK_CAP: //send 1
        case TYPE_DP_SNK_IDENTITY: //send 2
        case TYPE_SVID: //send 3
        case TYPE_SET_SNK_DP_CAP: //send 8
        case TYPE_DP_SNK_CFG://send 15
        case TYPE_BATT_STS://send 24
        case TYPE_ALERT://send 25   
        case TYPE_GET_COUNTRY_INFO://send 26
        case TYPE_GET_VAR://send 0xfc
        case TYPE_SET_VAR://send 0xfd
        case TYPE_CMD_SRC_CAP_1:
            l = Send_PD_Data_Messages(type, buf, size);
            break;
        case TYPE_PWR_OBJ_REQ://send 16
            if(Send_PD_Data_Messages(type, buf, size) == CMD_SUCCESS)
                l = CMD_SUCCESS;
            else
                l = CMD_FAIL;
            break;

        //Send VDM Messages
        case TYPE_VDM://send 14
            l =  send_vdm(type_sop, buf, size);
            break;
        case TYPE_I2C_MASTER_WRITE: //send 0x3e
            Send_Interface_Messages(type, buf, size);
            break;

        //Send Extend Messages 
        case TYPE_EXT_SRC_CAP_EXT: //send 30 //Added by ChengLin
        case TYPE_EXT_STS: //send 31 //Added by ChengLin
        case TYPE_EXT_SINK_CAP_EXT://send 3a //Added by ChengLin
        case TYPE_EXT_PPS_STS: //send 37 //Added by ChengLin
        case TYPE_EXT_GET_BATT_CAP://send 32   size = 1
        case TYPE_EXT_GET_BATT_STS://send 33
        case TYPE_EXT_BATT_CAP://send 34
        case TYPE_EXT_GET_MFR_INFO://send 35
        case TYPE_EXT_MFR_INFO://send 36
        case TYPE_EXT_COUNTRY_INFO://send 38
        case TYPE_EXT_COUNTRY_CODE://send 39
        case TYPE_EXT_PDFU_REQUEST://send 3b
            Send_PD_Extend_Messages(type,buf,size,type_sop);
            break;
        default:
            #ifdef ANXDEBUG
            ucsi_debug("unknown type %2X\n", type);
            #endif
            l = CMD_FAIL;
            break;
    }
#endif
      return l;
}


/**
 * @desc:   The Interface that AP handle the specific USB PD command from Ohio 
 *
 * @param: 
 *      type: PD message type, define enum PD_MSG_TYPE. 
 *      buf: the sepecific paramter pointer according to the message type:
 *                      eg: when AP update its source capability type=TYPE_PWR_SRC_CAP, 
 *          "buf" contains the content of PDO object,its format USB PD spec
 *                      customer can easily packeted it through PDO_FIXED_XXX macro:
 *                     default5Vsafe 5V, 0.9A fixed --> PDO_FIXED(5000,900, PDO_FIXED_FLAGS) 
 *                size: the paramter ponter's content length, if buf is null, it should be 0 
 *                  
 * @return:  0: success 1: fail 
 *  
 */ 
static u8 dispatch_rcvd_pd_msg()
{
    switch (recvd_msg_type()) {
        case TYPE_VDM: //0x14
            l = Recv_PD_VDM_Defalut_Callback();
            break;
        case TYPE_EXT_PDFU_RESPONSE: //0x3c
            break;
        case TYPE_RESPONSE_TO_REQ://0xf0
            l =  Recv_PD_Cmd_Rsp_Default_Callback();           
                break;
        case TYPE_GET_VAR://0xfc
            l = Recv_Debug_Callback();     
            break;
        default:
            l = Recv_PD_Commands_Default_Callback();
            break;
    }
    
    return l;
}


static void pd_cc_status_default_func(u8 cc_status)
{
    /* cc status */
    #ifdef ANXDEBUG
    ucsi_debug("cc status %x\n", (u16)cc_status);
    #endif
    context[port_id].power_op_mode = PO_USB_DEFAULT;
    role = get_data_role();
	context[port_id].anx_data_role = role;

    /* LBT-540 */
    anx_ucsi_cc_status_event(port_id, cc_status);

    if (role == 0)
        context[port_id].ucsi_connector_partner_type = CPT_DFP_ATTACHED;
    else
        context[port_id].ucsi_connector_partner_type = CPT_UFP_ATTACHED;

    switch ((cc_status & 0xF)) {
    case 1:
        switch (((cc_status >> 4) & 0xF)) {
        case 0:
        // CC1 Rd
            //UFP, CC2 Open
            context[port_id].ucsi_connector_partner_type = CPT_UFP_ATTACHED;
            break;
        case 1:
            //CC2 Rd, Debug
            context[port_id].ucsi_connector_partner_type = CPT_CAB_DEBUG_ACC;
            break;
        case 2:
            //CC2 Ra
            context[port_id].ucsi_connector_partner_type = CPT_CAB_UFP_ATTACHED;
            break;
        }
        break;
    case 2:
        switch (((cc_status >> 4) & 0xF)) {
        case 0:
        //CC1 Ra
            //UFP, CC2 Open
            context[port_id].ucsi_connector_partner_type = CPT_CAB_NO_UFP_ATTACHED;
            break;
        case 1:
            //CC2 Rd, Debug
            context[port_id].ucsi_connector_partner_type = CPT_CAB_UFP_ATTACHED;
            break;
        case 2:
            //CC2 Ra
            context[port_id].ucsi_connector_partner_type = CPT_AUDIO_ACC;
            break;
        }
        break;
    case 0:
        switch (((cc_status >> 4) & 0xF)) {
        case 1:
        //CC1 Open
            //CC2 Rd
            context[port_id].ucsi_connector_partner_type = CPT_UFP_ATTACHED;
            break;
        case 2:
                //CC2 Ra
            context[port_id].ucsi_connector_partner_type = CPT_CAB_NO_UFP_ATTACHED;
            break;
        case 0:
            //context[port_id].ucsi_connector_partner_type = CPT_RESERVED;
            break;
        }
        break;
    case 0x04:
        context[port_id].power_op_mode = PO_USB_DEFAULT;
        break;
    case 0x08:
        context[port_id].power_op_mode = PO_TYPE_C_1_5A;
        break;
    case 0x0c:
        context[port_id].power_op_mode = PO_TYPE_C_3A;
        break;
    }

    switch (((cc_status >> 4) & 0xF)) {
    case 0x04:
        context[port_id].power_op_mode = PO_USB_DEFAULT;
        break;
    case 0x08:
        context[port_id].power_op_mode = PO_TYPE_C_1_5A;
        break;
    case 0x0c:
        context[port_id].power_op_mode = PO_TYPE_C_3A;
        break;
    }

}

static u8 interface_send_msg()
{
    cac_checksum(context[port_id].InterfaceSendBuf, send_msg_len()+1);
    context[port_id].InterfaceSendBuf[send_msg_len() + 1] = 0 - j; //cmd + checksum
    len = send_msg_len() + 2;
    c = anx_read_reg(context[port_id].spi, InterfaceSendBuf_Addr);
    // retry
    if(c) {
        count = 250;
        while(count) { 
            mdelay(2);
            c = anx_read_reg(context[port_id].spi, InterfaceSendBuf_Addr);
            if (c == 0)
                break;
            count--;
        }
    }
    if (c == 0) {
        anx_write_block_reg(context[port_id].spi, InterfaceSendBuf_Addr+1 , len-1, &context[port_id].InterfaceSendBuf[1]);
        anx_write_reg(context[port_id].spi,InterfaceSendBuf_Addr, context[port_id].InterfaceSendBuf[0]);
    } else {
        #ifdef ANXDEBUG
        ucsi_debug("Tx Buf Full\n");
        #endif
        return CMD_FAIL;
    }

    return CMD_SUCCESS;
}

static u8 interface_send_ctr_msg()
{
    context[port_id].InterfaceSendBuf[0] = 1;
    context[port_id].InterfaceSendBuf[2] = 0-(context[port_id].InterfaceSendBuf[0]+context[port_id].InterfaceSendBuf[1]);//cac_checksum(InterfaceSendBuf, 1 + 1); //cmd + checksum
    len = 3;
    c = anx_read_reg(context[port_id].spi, InterfaceSendBuf_Addr);
    // retry
    if(c) {
        count = 250;
        while(count) { 
           mdelay(1);
           c = anx_read_reg(context[port_id].spi, InterfaceSendBuf_Addr);
           if (c == 0)
            break;
           count--;
        }
    }
    if (c == 0) {
        anx_write_block_reg(context[port_id].spi, InterfaceSendBuf_Addr+1 , len-1, &context[port_id].InterfaceSendBuf[1]);
        anx_write_reg(context[port_id].spi,InterfaceSendBuf_Addr, context[port_id].InterfaceSendBuf[0]);
    } else {
        #ifdef ANXDEBUG
        ucsi_debug("Tx Buf Full\n");
        #endif
        return CMD_FAIL;
    }

    return CMD_SUCCESS;
}

void dp_alt_enter_mode()
{
    Send_PD_Data_Messages(TYPE_DP_ALT_ENTER, enter_exit_svid, 4);
}

void dp_alt_exit_mode()
{
    Send_PD_Data_Messages(TYPE_DP_ALT_EXIT, enter_exit_svid, 4);
}

static u8 interface_recvd_msg()
{
    #ifdef ANX_I2C_BUFF_FAST_READ
    anx_read_block_reg(context[port_id].spi, InterfaceRecvBuf_Addr, 7, (u8 *)context[port_id].InterfaceRecvBuf);
    #else
    anx_read_block_reg(context[port_id].spi, InterfaceRecvBuf_Addr, 32, (u8 *)context[port_id].InterfaceRecvBuf);
    #endif
    if (context[port_id].InterfaceRecvBuf[0]!=0) {
        #ifdef ANX_I2C_BUFF_FAST_READ
        if (recvd_msg_len() > 4)
            anx_read_block_reg(context[port_id].spi, InterfaceRecvBuf_Addr+7, recvd_msg_len()-4, context[port_id].InterfaceRecvBuf + 7);
        #endif
        anx_write_reg(context[port_id].spi,InterfaceRecvBuf_Addr,0);
        j = 0;
        for(i = 0; i < recvd_msg_len() + 2 + 1; i++) {
            j += context[port_id].InterfaceRecvBuf[i];
        }
        if(j == 0) {
            return CMD_SUCCESS;
        } else {
            #ifdef ANXDEBUG
            ucsi_debug("checksum error: \n");
            #endif
        }
    }
    return CMD_FAIL;
}

static void handle_intr_vector()
{
    if (interface_recvd_msg() == CMD_SUCCESS)
    {
        #ifdef ANXDEBUG
        ucsi_debug("rev <- %s\n", interface_to_str((PD_MSG_TYPE)context[port_id].InterfaceRecvBuf[1]));
        #endif
        dispatch_rcvd_pd_msg();
    }
}

#ifdef PATCH_NOT_CALIBRATED_CHIP
#define ANALOG_CTRL_6                           0xA6
#define ADC_CTRL_2                              0xC0
#define EFUSE_CTRL_10                           0x7C
#define EFUSE_CTRL_11                           0x7D
#define EFUSE_PROGRAM_DONE_CLR _BIT7
#define EFUSE_READ_DONE_CLR _BIT6
#define EFUSE_READ_DONE _BIT4
#define EFUSE_READ_DATA_0                       0x8D
void eFuseRead(unsigned char offset, unsigned char len, unsigned char *buf)
{
	anx_read_block_reg(context[port_id].spi,EFUSE_READ_DATA_0+offset,len,buf);
}
unsigned char isCalibrated()
{
  unsigned char buf[3];
  eFuseRead(0,3,buf);
  if(!buf[0] || !buf[1] || !buf[2])
  	return 0;
  return 1;
}
#endif
static u8 wait_ocm_ready(u8 fource_use_pd_20)
{
    count = 3200;

    do {
        count--;
    } while(!(anx_read_reg(context[port_id].spi, R_RAM_CTRL)&BOOT_LOAD_DONE));

   #ifdef PATCH_NOT_CALIBRATED_CHIP
	c = anx_read_reg(context[port_id].spi, 0x95);
   if(!isCalibrated())
   {
   #ifdef ANXDEBUG
   	ucsi_debug("Not calibrate chip.\n");
   #endif
	 anx_write_reg(context[port_id].addr, RING_OSC_CTRL, 0xb7); //27M clock
       if (c >= 0xAC && c != 0xFF) {
   	 anx_write_reg(context[port_id].addr, ADC_CTRL_2, 0x26);  ////adc                AA: 0x0f    AC:0x26
       }else{
    anx_write_reg(context[port_id].addr, ADC_CTRL_2, 0x0f);  //adc
	}
    anx_write_reg(context[port_id].addr, ANALOG_CTRL_6, 0x11);  //5.1K
   }
   #endif
    // configure OCM init use PD2.0 
    if(fource_use_pd_20)
        anx_write_reg(context[port_id].spi, PD_REV_INIT, 0x01);
    else
        anx_write_reg(context[port_id].spi, PD_REV_INIT, 0x00);

    do{
        if(!count)
        {
            #ifdef ANXDEBUG
            ucsi_debug("ocm not ready!\n");
            #endif
            break;
        }
        count--;
        k = anx_read_reg(context[port_id].spi, OCM_VERSION_REG);
    } while((k == 0));

#ifdef ANXDEBUG
    ucsi_debug("ocm ready(%d)!\n",(3200-count));
#endif
   
    return((k & BOOT_LOAD_DONE) ? 1:0);
}
code unsigned char pdss_src_pdo[4] = {
    /*5V 1.5A dual-role data-swap usb-comm */
    0x96,0x90,0x01,0x26
};

void send_initialized_setting()
{
	RamDebug(0x77);
    switch (context[port_id].anx_initial_status) {
    case INIT_STAGE_1:
#ifdef ANXDEBUG
        ucsi_debug("Sending srouce capability\n");
#endif
        //send_source_capability();
        context[port_id].anx_initial_status++;
        break;
   case INIT_STAGE_2:
        #ifdef ANXDEBUG
        ucsi_debug("Sending sink capability\n");
        #endif

        //send_sink_capability();
        context[port_id].anx_initial_status++;
        break;
    case INIT_STAGE_3:
        #ifdef ANXDEBUG
        ucsi_debug("Sending sink identity\n");
        #endif

        /* send TYPE_DP_SNK_IDENTITY init setting */
        send_pd_msg(TYPE_DP_SNK_IDENTITY, &snk_identity,
               sizeof(snk_identity), SOP_TYPE);
        context[port_id].anx_initial_status++;
        break;
    case INIT_STAGE_4:
        #ifdef ANXDEBUG
        ucsi_debug("Sending svid\n");
        #endif
        context[port_id].anx_initial_status++;
        break;
    case INIT_STAGE_5:
        //send_pd_msg(TYPE_SET_VAR,set_variable_1,5,0);
        //send_pd_msg(TYPE_SET_SNK_DP_CAP, dp_source_dp_caps, 4, SOP_TYPE);
        context[port_id].anx_initial_status++;
        break;
    case 55:
        context[port_id].anx_initial_status++;
        RamDebug(0x20);
        #ifdef ANXDEBUG
        ucsi_debug("___________  data operation mode is %.bx.\n",
            context[port_id].data_operation_mode_uom);
        #endif
        RamDebug(context[port_id].data_operation_mode_uom);
        #if 1
        if ((context[port_id].data_operation_mode_uom == 0) ||
                (context[port_id].data_operation_mode_uom & OP_DRP))
            break;
        switch (get_data_role()) {
        case 1: //DFP
            role = OP_DFP;
            RamDebug(0x1);
            break;
        case 0: //UFP
            role = OP_UFP;
            RamDebug(0x2);
            break;
        }
        if (context[port_id].data_operation_mode_uom & role)
            break;
        RamDebug(0x40);
        //send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
        /* fix JIRA LBT-552 */
        chip_power_down(port_id);
        anx_initial_context(port_id);
        #endif
        break;
    case 56:
        context[port_id].anx_initial_status ++;
        #ifdef ANXDEBUG
        ucsi_debug("___________  power operation mode is %.bx.\n",
            context[port_id].power_operation_mode_pdm);
        #endif
        if ((context[port_id].power_operation_mode_pdm == 0) ||
                (context[port_id].power_operation_mode_pdm & PW_DRP))
            break;
        switch (get_power_role()) {
        case 1: //PW_PROVIDER
            role = PW_PROVIDER;
            break;
        case 0: //PW_CONSUMER
            role = PW_CONSUMER;
            break;
        }
        if (context[port_id].power_operation_mode_pdm & role)
            break;
        send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
        break;
    case 80:
        context[port_id].anx_initial_status = 0;
        //Fix JIRA LBT-404
        #ifdef ANX_SEND_GET_SOURCE_CAPS
        if (request_src_caps_flag[port_id] == 1) {
            request_src_caps_flag[port_id] = 0;
            send_pd_msg(TYPE_GET_SRC_CAP, 0, 0, SOP_TYPE);
			RamDebug(0x88);
        }
        #endif
        context[port_id].enable_report_power_on = 1;
        break;
    default:
        context[port_id].anx_initial_status++;
        break;
    case 0:
        break;
    }

}

static void drp_toggle_enable() 
{     
	c = anx_read_reg(context[port_id].spi, 0x95);
    if (context[port_id].data_operation_mode_uom & OP_DFP) {
        anx_write_reg(context[port_id].addr, TCPC_ROLE_CONTROL, 0x5); //CC with  Rp + 1.5A
        #ifdef ANXDEBUG
        ucsi_debug("-------  set tcpc role as Rp(0x5), reg 0xaa(%.bx)\n", c);
        #endif
        RamDebug(0xdd);
    } else if (context[port_id].data_operation_mode_uom & OP_UFP) {
        anx_write_reg(context[port_id].addr, TCPC_ROLE_CONTROL, 0xA); //CC with  RD
        #ifdef ANXDEBUG
        ucsi_debug("-------  set tcpc role as Rd(0xa), reg 0xaa(%.bx)\n", c);
        #endif
        RamDebug(0xdc);
    } else {
        RamDebug(0xdb);
        anx_write_reg(context[port_id].addr, ANALOG_CTRL_10, 0x80);
        anx_write_reg(context[port_id].addr, TCPC_ROLE_CONTROL, 0x4A); //CC with  RD
        #ifdef ANXDEBUG
        ucsi_debug("-------  set tcpc role as Rd + DRP en(0x4A), %.bx chip\n", c);
        #endif
       if (c >= 0xAC && c != 0xFF) {     
    		anx_write_reg(context[port_id].addr,TCPC_COMMAND , 0x99);  //DRP en
      	} else {
            anx_write_reg(context[port_id].addr, ANALOG_CTRL_1, 
                (anx_read_reg(context[port_id].addr, ANALOG_CTRL_1) | TOGGLE_CTRL_MODE | R_TOGGLE_ENABLE)); 
            anx_write_reg(context[port_id].addr, ANALOG_CTRL_1,
                (anx_read_reg(context[port_id].addr, ANALOG_CTRL_1) | R_LATCH_TOGGLE_ENABLE)); 
   	    }
   	}
}

static void pd_ovp_set()
{
    /* disable VBUS protection */
    anx_write_reg(context[port_id].addr, 0xcd, 0x80);
    count = (PD_MAX_VOLTAGE * (100/25)) + 32;
    anx_write_reg(context[port_id].addr, 0x76, count & 0xFF);
    anx_write_reg(context[port_id].addr, 0x77, (count >> 8) & 0xFF);

    /* clear interrupt */
    anx_write_reg(context[port_id].addr, 0x10, 0x80);

    /* enable VBUS protection */
    anx_write_reg(context[port_id].addr, 0xcd, 0xA0);
}

static void usb_pd_function_init()
{
    wait_ocm_ready(0);

    #ifdef ANX_OVP_RE_SET
    pd_ovp_set();
    #endif
    //anx_write_reg(context[port_id].spi, MAX_VOLTAGE, PD_MAX_VOLTAGE);
    //anx_write_reg(context[port_id].spi, MAX_POWER, PD_MAX_POWER);
	//J80_018-->>
	/*
    anx_write_reg(context[0].spi, MAX_VOLTAGE, PD_MAX_VOLTAGE);
    anx_write_reg(context[0].spi, MAX_POWER, PD_MAX_POWER);
	*/
	//J80_018--<<
	//J80_018++>>
	anx_write_reg(context[0].spi, MAX_VOLTAGE, 0x32);
    anx_write_reg(context[0].spi, MAX_POWER, 0x1E);
	//J80_018++<<
	#if Two_PD_Chip
	anx_write_reg(context[1].spi, MAX_VOLTAGE, 0x32); 	// 5V
    anx_write_reg(context[1].spi, MAX_POWER, 0x1E);		// 15W
	#endif

    /*Minimum Power in 500mW units*/
    anx_write_reg(context[port_id].spi, MIN_POWER, PD_MIN_POWER);
    #if (PD_MAX_INSTANCE == 2)
    if (context[0].anx_power_status == 0 && context[1].anx_power_status == 0)
        random_data = (timer1msCount % 256);
    anx_write_reg(context[port_id].addr, 0xef, random_data);
    #endif
    k = 0;
    /*
        * bit 0: enable unstructure VDM(fix self charging) 
        * bit 1: dead battery mode, OCM maximum delay 200ms
        * bit 3 & 2 : 00:vsafe 0v, 01:vsafe 1v, 10:vsafe 2v, 11:vsafe 3v
        */
    k = k | 0x3;
	k = k | (0x1 << 2);
    if (anx_read_reg(context[port_id].spi, OCM_VERSION_REG) < 0x11)
        anx_write_reg(context[port_id].addr, 0xee, k);
    else
        anx_write_reg(context[port_id].addr, 0xf0, k);

    k = anx_read_reg(context[port_id].spi, FIRMWARE_CTRL);
    /* Default enable auto pd mode */
    #ifdef ANXDEBUG
    ucsi_debug("enable auto pd function\n");
    #endif
    #ifdef AUTO_RDO_ENABLE
    k = k | auto_pd_en | high_voltage_for_same_power;
    #endif
    //Fix JIRA LBT-366
    k = k | 0x40;
    // trySrc_trySnk_setting
    k = k & (~(trysrc_en | trysnk_en));

    if((!context[port_id].trysnk_enable)&&(context[port_id].trysrc_enable)) {
        //k = k | trysrc_en;
        #ifdef ANXDEBUG
        //ucsi_debug("Prefer Try.Src\n");
        #endif
    } else if((!context[port_id].trysrc_enable) &&(context[port_id].trysnk_enable)) {
        //k = k | trysnk_en;
        #ifdef ANXDEBUG
        //ucsi_debug("Prefer Try.Snk\n");
        #endif
    } 

    anx_write_reg(context[port_id].spi, FIRMWARE_CTRL, k);
    //Disable OVP
    anx_write_reg(context[port_id].addr, 0xdd, 0xff);
    anx_write_reg(context[port_id].addr, 0xde, 0x03);
    //interrupt mask initial
    mdelay(2);
    /* LBT-539 */
    anx_write_reg(context[port_id].spi, INT_MASK,
        (anx_read_reg(context[port_id].spi, INT_MASK) &
        (~CC_STATUS_CHANGE) & (~PR_CONSUMER_GOT_POWER)));

    // config OCM reply message
    anx_write_reg(context[port_id].spi, PD_EXT_MSG_CTRL, 0xFF);

}
void chip_power_on(u8 port)
{
    anx_read_reg(context[port].addr,TCPC_COMMAND);
    RamDebug(0xE3);
    #ifdef ANXDEBUG
    ucsi_debug("-------  ppm_init power on port %.bx.\n", port);
    #endif
}
void chip_power_down(u8 port) 
{
    if (port == 0)
        RamDebug(0xE1);
    else
        RamDebug(0xF1);
    anx_write_reg(context[port].spi, OCM_CTRL_0,
        (anx_read_reg(context[port].spi, OCM_CTRL_0) | OCM_RESET));
    drp_toggle_enable() ;
    anx_write_reg(context[port].addr, TCPC_COMMAND , 0xff);  //lpower down
}

#ifdef ECR_CHECKING
static void anx_ecr_checking(void)
{
    if (((context[port_id].ucsi_partner_rdo >> 10) & 0x3ff) == (PD_CURRENT_1500MA / 10)) {
        supply_1500ma_flag = 0;
        context[port_id].ucsi_partner_rdo = 0;
    }
}
#endif

static void anx_detect_voltage_and_current(u8 port){
	change_status = anx_read_reg(context[port].spi, SYSTEM_STSTUS);
    change_int = anx_read_reg(context[port_id].spi, CHANGE_INT);
    #ifdef ANXDEBUG
    ucsi_debug("port[%.bx], system_status[%.bx], int_status[%.bx].\n",
        port, change_status, change_int);
    #endif
    if (change_status & _BIT4) {
        self_charging_error = 1;
        return;
    }

	if (((change_status & VBUS_STATUS) == 0) || (change_int & PR_CONSUMER_GOT_POWER)) {
		context[port].anx_power_role = 0;
        context[port].BatteryCharging = BCS_NOMINAL_CHARGING;
		context[port].anx_current_in_50ma = anx_read_reg(context[port].spi, 0xb1);
		context[port].anx_voltage_in_100mv = anx_read_reg(context[port].spi, 0xaf);
		if ((context[port].anx_current_in_50ma == 0) || (context[port].anx_voltage_in_100mv == 0)) {
			change_status = anx_read_reg(context[port].spi, NEW_CC_STATUS);
			pd_cc_status_default_func(change_status);
			context[port].anx_voltage_in_100mv = 50;
			switch (context[port].power_op_mode) {
			default:
			case PO_USB_DEFAULT:
				context[port].anx_current_in_50ma = 10;
				break;
			case PO_TYPE_C_1_5A:
				context[port].anx_current_in_50ma = 30;
				break;
			case PO_TYPE_C_3A:
				context[port].anx_current_in_50ma = 60;
				break;
			}
		}
	} else {
        context[port].BatteryCharging = BCS_NOT_CHARGING;
		context[port].anx_power_role = 1;
	}
}
void anx_dead_battery_checking_2(u8 port)
{
    if (context[port].anx_power_status == 0) {
		change_status = anx_read_reg(context[port].spi, NEW_CC_STATUS);
		if (change_status == 0)
			return; //return if no cc connection
			context[port].anx_power_status = 1;
		}
	anx_detect_voltage_and_current(port);
}

void anx_dead_battery_checking_1()
{
    for (i = 0; i < PD_MAX_INSTANCE; i++) {
        if (context[i].anx_power_status == 0) {
            change_status = anx_read_reg(context[i].spi, NEW_CC_STATUS);
            if (change_status != 0) {
                context[i].anx_power_status = 1;
                pd_cc_status_default_func(change_status);
                change_status = anx_read_reg(context[i].spi, SYSTEM_STSTUS);
                if (change_status & VBUS_CHANGE)
                    context[i].anx_power_role = 1;
                else
                    //debug only
                    context[i].anx_power_role = 0;

                if (change_status & _BIT4) {
                    self_charging_error = 1;
                    return;
                }

                if (context[i].anx_power_role == 0) {
                    context[i].anx_current_in_50ma = anx_read_reg(context[port_id].spi, 0xb1);
                    context[i].anx_voltage_in_100mv = anx_read_reg(context[port_id].spi, 0xaf);
                    if (context[i].anx_current_in_50ma == 0 ||
                            context[i].anx_voltage_in_100mv == 0) {
                        context[i].anx_voltage_in_100mv = 50;
                        switch (context[port_id].power_op_mode) {
                        default:
                        case PO_USB_DEFAULT:
                            context[i].anx_current_in_50ma = 10;
                            break;
                        case PO_TYPE_C_1_5A:
                            context[i].anx_current_in_50ma = 30;
                            break;
                        case PO_TYPE_C_3A:
                            context[i].anx_current_in_50ma = 60;
                            break;
                        }
                    }
                }
                #ifdef ANXDEBUG
                ucsi_debug("%s:%d  --- voltage : %.bx, current : %.bx.\n",
                    __func__, __LINE__, context[i].anx_voltage_in_100mv, context[i].anx_current_in_50ma);
                #endif
            }
        }
    }
}
void anx_alert_message_isr()
{
    if (0 == context[port_id].anx_power_status) {
        if (anx_read_reg(context[port_id].spi, OCM_VERSION_REG) == 0x00) {
			anx_write_reg(context[port_id].addr, 0x10, 0xFF);
			anx_write_reg(context[port_id].addr, 0x11, 0xFF);
			return;
        }
	}

    /* Fix JIRA LBT-501 */
    anx_read_block_reg(context[port_id].spi, CHANGE_INT, 2, &change_int);
    //change_status = anx_read_reg(context[port_id].spi, SYSTEM_STSTUS);
    anx_read_block_reg(context[port_id].addr, INTR_ALERT_0, 2, &intr_alert_0);
    //intr_alert_1 = anx_read_reg(context[port_id].addr, INTR_ALERT_1);
    //clear
    anx_write_reg(context[port_id].spi, CHANGE_INT, 0x00);
    anx_write_block_reg(context[port_id].addr, INTR_ALERT_0, 2, &intr_alert_0);
    //anx_write_reg(context[port_id].addr, INTR_ALERT_1, intr_alert_1);

    #ifdef ANXDEBUG
    //ucsi_debug("\nalert 1 register is %x, change_status %x.\n", (u16)intr_alert_1, (u16)change_status);
    #endif
    /* power on process */
    if(intr_alert_1 & INTR_INTP_POW_ON) {
        usb_pd_function_init();
        j = anx_read_reg(context[port_id].spi, OCM_VERSION_REG);
        k = anx_read_reg(context[port_id].spi, OCM_VERSION_REG + 1);
        if (port_id == 0) {
            xEC_PDPort0MainVersion = j;
            xEC_PDPort0SubVersion = k;
        } else {
            xEC_PDPort1MainVersion = j;
            xEC_PDPort1SubVersion = k;
        }
        #ifdef ANXDEBUG
        ucsi_debug("\nliberty (port %.bx) power on, ocm version v%bx.%bx.%bx.\n",
            port_id, (j>>4)&0xf, j&0xf, k);
        #endif

        //send_initialized_setting();
        anx_initial_context(port_id);
        context[port_id].anx_power_status = 1;
        context[port_id].csc.csc.Connect = 1;
        context[port_id].ConnectorPartner = CP_USB;
        context[port_id].csc.csc.ConnectorPartner = 1;
        context[port_id].csc.csc.BatteryChargingStatus = 1;
        context[port_id].anx_initial_status = INIT_STAGE_1;
        //Fix JIRA LBT-215
		send_pd_msg(TYPE_SVID, &svid, sizeof(svid), SOP_TYPE);
		send_pd_msg(TYPE_SET_VAR,set_variable_1,5,0);
        //Fix JIRA LBT-411
		send_pd_msg(TYPE_SET_SNK_DP_CAP, dp_source_dp_caps, 4, SOP_TYPE);
        //Fix JIRA LBT-414
		send_source_capability();
		send_sink_capability();
        anx_ucsi_pin_assignment_event(port_id, SELECT_PIN_ASSIGMENT_U);
        //send_initialized_setting();
	    ucsi_async_notify_raise_up(150);
    }
    if (intr_alert_1 & INTR_INTP_POW_OFF) {
        if (port_id == 0)
            RamDebug(0xE2);
        else
            RamDebug(0xF2);
        chip_power_down(port_id);
        #ifdef ANXDEBUG
        ucsi_debug("\nliberty(port %.bx) interrupt power off, alert0(%.bx), alert1(%.bx).\n",
            port_id, intr_alert_0, intr_alert_1);
        #endif
        context[port_id].anx_power_status = 0;
        anx_initial_context(port_id);
        context[port_id].csc.csc.Connect = 1;
        ucsi_async_notify_raise_up(2);
        //anx_vbus_ctrl(context[port_id].vbus_gpio, 0);
        #ifdef ECR_CHECKING
        anx_ecr_checking();
        #endif
        context[port_id].anx_vbus_status = 0;
        anx_ucsi_detach_event(port_id);
        return;
    }

    /*Received interface message*/
    if (intr_alert_0 & INTR_RECEIVED_MSG) {
        handle_intr_vector();
    }
    /*Received software interrupt*/
    if (intr_alert_0 & INTR_SOFTWARE_INT) {
        
        if (change_int & HPD_STATUS_CHANGE) {
            context[port_id].hpd_status = change_status & HPD_STATUS;
            /* LBT-540 */
            anx_ucsi_hpd_event(port_id, context[port_id].hpd_status);
        }

        if (change_int & VBUS_CHANGE) {
            context[port_id].csc.csc.PowerDirection = 1;
            if (change_status & VBUS_CHANGE)
            {
                context[port_id].anx_vbus_status = 1;
                context[port_id].anx_power_role = 1;
                context[port_id].csc.csc.BatteryChargingStatus = 1;
                context[port_id].BatteryCharging = BCS_NOT_CHARGING;
                #ifdef ANXDEBUG
                ucsi_debug("VBUS 5V OUT.\n");
                #endif
            }
            else
            {
                //DISABLE_5V_VBUS_OUT();
                #ifdef ANXDEBUG
                ucsi_debug("VBUS change to input.\n");
                #endif
                #ifdef SUP_VBUS_CTL
                //anx_vbus_ctrl(context[port_id].vbus_gpio, 0);
                #endif
                context[port_id].anx_power_role = 0;
                context[port_id].anx_vbus_status = 0;
                #ifdef ReportExternalSupply
                context[port_id].csc.csc.ExternalSupply = 1;
                #endif
                context[port_id].csc.csc.BatteryChargingStatus = 1;
                context[port_id].BatteryCharging = BCS_NOMINAL_CHARGING;
            }
            ucsi_async_notify_raise_up(100);
        }
        if (change_int & CC_STATUS_CHANGE) {
            change_status = anx_read_reg(context[port_id].spi, NEW_CC_STATUS);
            if(plug_orientation(port_id))
                context[port_id].cc_orientation = CC1_CONNECTED;
            else
                context[port_id].cc_orientation = CC2_CONNECTED;
            /* LBT-540  */
            pd_cc_status_default_func(change_status);
            #ifdef ANXREDRIVER
            if (context[port_id].pd_capability & PD_HAS_REDRIVER) {
                if(context[port_id].mux_mode == NO_CONNECT) { //usb3.0 only mode at default
                    redriver_mux_control(USB3_1_CONNECTED, context[port_id].cc_orientation);
                    context[port_id].mux_mode = USB3_1_CONNECTED;
                }
            }
            #endif
        }
    }
    
    anx_dead_battery_checking_2(port_id);
}

static void anx7447_context_initial()
{
    //Addr select as : 0 0
    context[port_id].PD_standalone_slave_id1 = Liberty_i2c_address[port_id][0];
    context[port_id].PD_SPI_slave_id = Liberty_i2c_address[port_id][1];
    context[port_id].PD_EMTB_slave_id = Liberty_i2c_address[port_id][2];
    context[port_id].PD_EMRB_slave_id = Liberty_i2c_address[port_id][3];

    //context[port_id].alert_gpio = 1;
    #ifdef ANX_OHO_DFP_ONLY
    context[port_id].dfp_only = 1;
    #else
    context[port_id].dfp_only = 0;
    #endif
    context[port_id].pd_src_pdo_cnt = 2;
    context[port_id].anx_power_status = 0;
    context[port_id].sel_voltage_pdo_index = 0x2;

    context[port_id].pd_capability = PD_HAS_RETURN_STATUS | PD_HAS_SVID_MID | PD_HAS_CABLE_VDO;
    #ifdef ANXREDRIVER
    if (port_id == 1) {
        context[port_id].pd_capability |= PD_HAS_REDRIVER;
    }
    #endif

    context[port_id].error_count = 0;
}

static void anx7447_pup_initial()
{
    pup.alt_mode[0].SVID0 = 0xFF01; 
    pup.alt_mode[0].MID0 = 0x00000405; 
    pup.alt_mode_support_num = 1;
    pup.alt_mode_support_bitmap = 0;
    pup.current_alt_mode = 0;
}

/*
 * @desc   : ANX7447 ucsi module initialize all data structure.
 * @param  : none
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
void anx7447_ucsi_init()
{
#ifdef ANXDEBUG
    ucsi_debug("************************************************************\n");
#endif
    //ucsi_debug("* %s:%d anx initialization, version %d.%d . *\n", __func__, __LINE__,
    //  ANX_UCSI_VERSION_MAJOR, ANX_UCSI_VERSION_MINOR);
    i = 0;
    anx7447_pup_initial();
    for (i = 0; i < PD_MAX_INSTANCE; i++) {
        port_id = i;
        anx7447_context_initial();
        context[i].InterfaceRecvBuf = &InterfaceRecvBuf[i];
        context[i].InterfaceSendBuf = &InterfaceSendBuf[i];
        context[i].connector_index = i+1;
    }

}

