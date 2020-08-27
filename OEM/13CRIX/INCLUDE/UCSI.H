/******************************************************************************
Copyright (c) 2017, Analogix Semiconductor, Inc.

PKG Ver  : V1.1.1
Filename : ucsi.h
Project  : EC UCSI Multiport
Created  : 17 Jul 2017
Devices  : ANX7447
Toolchain: C51

Description:
Define the basic UCSI data structure, macro and other initial interface.

******************************************************************************/
#ifndef _UCSI_H_
#define _UCSI_H_

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

#ifndef __func__
#define __func__    __FILE__
#endif

typedef unsigned char     bool;
typedef unsigned char     u8;
typedef char              s8;
typedef unsigned int      u16;
typedef unsigned long     u32;

/*
typedef XBYTE            u8;
typedef XBYTE            s8;
typedef XBYTE            bool;
typedef XWORD            u16;
typedef XWORD            u32;
*/
typedef struct {
    u32 word1;
    u32 word2;
} u64;

#define EC_ANX_BIG_ENDIAN
extern u8 SWAP_BYTE;
#define SWAP_WORD(W) do {\
    SWAP_BYTE = ((u8 *)W)[0];\
    ((u8 *)W)[0] = ((u8 *)W)[1];\
    ((u8 *)W)[1] = SWAP_BYTE;\
} while(0)

#define SWAP_DWORD(DW) do {\
    SWAP_BYTE = ((u8 *)DW)[0];\
    ((u8 *)DW)[0] = ((u8 *)DW)[3];\
    ((u8 *)DW)[3] = SWAP_BYTE;\
    SWAP_BYTE = ((u8 *)DW)[1];\
    ((u8 *)DW)[1] = ((u8 *)DW)[2];\
    ((u8 *)DW)[2] = SWAP_BYTE;\
} while (0)

#define SWAP_DWORD_WORD(DWW) {\
    SWAP_WORD(DWW);\
    SWAP_WORD(DWW+2);\
}


#define ANXREENTRANT reentrant
#define ANXXDATA    xdata
#define ANXXDATALOCAL
#define UNREFERENCE(X)  (X = X)
#define ANXINLINE
#define ANXPACK

/*enable ANX debug mode*/
#if UART_Debug

/*set the INT and Address configuration for ANX EVB*/
//#define ITE_EC_EVB_PORT1_TEST


#define ANXDEBUG
#define ucsi_debug printf
#endif

void ANX_HOOK_10ms(void);

void ANX_HOOK_1ms(void);


typedef void (*ec_caller)(u8 param);

/*
 * @desc   : I2C read one register
 * @param  :
 *           i2c_addr -> i2c slave address
 *           reg -> register offset
 * @return :
 *           return register value if success
 */
#define anx_read_reg i2c_read_reg
//u8 anx_read_reg(u8 i2c_addr, u8 reg);

/*
 * @desc   : I2C write one register
 * @param  :
 *           i2c_addr -> i2c slave address
 *           reg -> register offset
 *           data -> value which will be wrote
 * @return : none
 */
//void anx_write_reg(u8 i2c_addr, u8 reg, u8 data1);
#define anx_write_reg i2c_write_reg

/*
 * @desc   : I2C read block registers
 * @param  :
 *           i2c_addr -> i2c slave address
 *           reg -> register offset
 *           length -> the amount of read register
 *           buf -> value stored to
 * @return : none
 */
//void anx_read_block_reg(u8 i2c_addr, u8 reg, u8 length, u8 *buf);
#define anx_read_block_reg i2c_read_block_reg

/*
 * @desc   : I2C write block registers
 * @param  :
 *           i2c_addr -> i2c slave address
 *           reg -> register offset
 *           length -> the amount of read register
 *           buf -> value stored to
 * @return : none
 */
//void anx_write_block_reg(u8 i2c_addr, u8 reg, u8 length, u8 *buf);
#define anx_write_block_reg i2c_write_block_reg_b

/*
 * @desc   : timer schedule function
 * @param  :
 *           func - function pointer
 *           time - timeout value in ms
 * @return : none
 */
//void timer_scheduler(ec_caller func, u8 port_index, u16 time);

/* millisecond delay */
//void mdelay(u32 ms);
#define mdelay DelayXms

/* checking A is before B or not */
#define time_before(A, B) (((B-A)>0)?1:0)
//u8 time_before(u32 A, u32 B);

#define __UCSI__
#define __PPM__

extern struct bios_share_msg ANXXDATA ppm2bios;
/*
 * Shared Message Data Structure
 */
struct bios_share_msg {
    u16 version;
    u16 reserved;
    u32 CCI;
    u64 CONTROL;
    u8  msgi[16];
    u8  msgo[16];
} ANXPACK;

/*
 * USB Type-C Command Status and Connector Change Indication Data Structure.
 */
struct cci_status {
    u8 Reserved1                   : 1;
    u8 ConnectorChangeIndicator    : 7;
    u8 DataLength;
    u8 Reserved2;
    u8 Reserved3                   : 1;
    u8 NotSupportedIndicator       : 1;
    u8 CancelCompletedIndicator    : 1;
    u8 ResetCompletedIndicator     : 1;
    u8 BusyIndicator               : 1;
    u8 AcknowledgeCommandIndicator : 1;
    u8 ErrorIndicator              : 1;
    u8 CommandCompletedIndicator   : 1;
} ANXPACK;

/*
 * USB Type-C Control Data Structure header.
 */
struct control_header {
    u8 Command;
    u8 DataLength;
} ANXPACK;


/*
 * Command Code
 */
#define UCSI_COMMAND_RESERVED               0x00
#define UCSI_COMMAND_PPM_RESET              0x01
#define UCSI_COMMAND_CANCEL                 0x02
#define UCSI_COMMAND_CONNECTOR_RESET        0x03
#define UCSI_COMMAND_ACK_CC_CI              0x04
#define UCSI_COMMAND_NOTIFICATION_ENABLE    0x05
#define UCSI_COMMAND_CAPABILITY             0x06
#define UCSI_COMMAND_CONNECTOR_CAPABILITY   0x07
#define UCSI_COMMAND_UOM                    0x08
#define UCSI_COMMAND_UOR                    0x09
#define UCSI_COMMAND_PDM                    0x0A
#define UCSI_COMMAND_PDR                    0x0B
#define UCSI_COMMAND_GET_ALTERNATE_MODES    0x0C
#define UCSI_COMMAND_GET_CAM_SUPPORTED      0x0D
#define UCSI_COMMAND_GET_CURRENT_CAM        0x0E
#define UCSI_COMMAND_SET_NEW_CAM            0x0F
#define UCSI_COMMAND_GET_PDOS               0x10
#define UCSI_COMMAND_GET_CABLE_PROPERTY     0x11
#define UCSI_COMMAND_GET_CONNECTOR_STATUS   0x12
#define UCSI_COMMAND_GET_ERROR_STATUS       0x13

#define UCSI_COMMAND_SUCC       1
#define UCSI_COMMAND_FAIL       2
#define UCSI_COMMAND_NOT_NOTIFY 3
//#define UCSI_COMMAND_BUSY       4
#define UCSI_COMMAND_CONTINUE   5
#define UCSI_COMMAND_RETURN_BUSY_ACK        0x1

/*
 * Parameter values
 */
#define UCSI_MAX_DATA_LENGTH                0x10
#define UCSI_MAX_NUM_ALT_MODE               0x80
#define UCSI_MIN_TIME_TO_RESPOND_WITH_BUSY  0x0A
#define UCSI_GET_ERROR_STATUS_DATA_LENGTH   0x10

/*
 * PPM_RESET Command
 */
struct reset {
    struct control_header header;
    u16 Reserved1;
    u32 Reserved2;
} ANXPACK;

/*
 * CANCEL Command
 */
struct cancel {
    struct control_header header;
    u16 Reserved1;
    u32 Reserved2;
} ANXPACK;

/*
 * CONNECTOR_RESET Command
 */
struct connector_reset {
    struct control_header header;
    u8 ConnectorNumber         : 7;
    u8 HardReset               : 1;
    u8 Reserved1               : 8;
    u32 Reserved2;
} ANXPACK;

/*
 * ACK_CC_CI Command
 */
struct ack_cc_ci {
    struct control_header header;
    u8 ConnectorChangeAcknowledge      : 1;
    u8 CommandCompletedAcknowledge     : 1;
    u8 Reserved1                       : 6;
    u8 Reserved2;
    u32 Reserved3;
} ANXPACK;

/*
 * SET_NOTIFICATION_ENABLE status
 */
struct notification_enable_status {
    u8 CommandCompleted                    : 1;
    u8 ExternalSupplyChange                : 1;
    u8 PowerOperationModeChange            : 1;
    u8 Reserved1                           : 1;
    u8 Reserved2                           : 1;
    u8 SupportedProviderCapabilitiesChange : 1;
    u8 NegotiatedPowerLevelChange          : 1;
    u8 PDResetComplete                     : 1;
    u8 SupportedCAMChange                  : 1;
    u8 BatteryChargingStatusChange         : 1;
    u8 Reserved3                           : 1;
    u8 ConnectorPartnerChange              : 1;
    u8 PowerDirectionChange                : 1;
    u8 Reserved4                           : 1;
    u8 ConnectChange                       : 1;
    u8 Error                               : 1;
} ANXPACK;
extern struct notification_enable_status ne;

/*
 * SET_NOTIFICATION_ENABLE Command
 */
struct set_notification_enable {
    struct control_header header;
    struct notification_enable_status status;
    u32 Reserved5;
} ANXPACK;

/*
 * GET_CAPABILITY Command
 */
struct get_capability {
    struct control_header header;
    u16 Reserved1;
    u32 Reserved2;
} ANXPACK;

/*
 * GET_CAPABILITY DATA
 */
struct capability_data {
    union {
        u32 v0;
        struct {
            u8 DisabledStateSupport : 1;
            u8 BatteryChaging       : 1;
            u8 USBPowerDelivery     : 1;
            u8 Reserved1            : 3;
            u8 USBTypeCCurrent      : 1;
            u8 Reserved2            : 1;
            union {
                u8 v0;
                struct {
                    u8 ACSupply     : 1;
                    u8 Reserved3    : 1;
                    u8 Other        : 1;
                    u8 Reserved4    : 3;
                    u8 UsesVBUS     : 1;
                    u8 Reserved5    : 1;
                } v1;
            } bmPowerSource;
            u16 Reserved6;
        } v1;
    } bmAttributes;
    u8 bNumConnectors               : 7;
    u8 Reserved7                    : 1;
    u8 bmOptionalFeatures1;
    u8 bmOptionalFeatures2;
    u8 bmOptionalFeatures3;
    u8 bNumAltModes;
    u8 Reserved8;
    u16 bcdBCVersion;
    u16 bcdPDVersion;
    u16 bcdUSBTypeCVersion;
} ANXPACK;


/*
 * GET_CONNECTOR_CAPABILITY Command
 */
struct get_connector_capability {
    struct control_header header;
    u8 ConnectorNumber             : 7;
    u8 Reserved1                   : 1;
    u8 Reserved2;
    u32 Reserved3;
} ANXPACK;

/*
 * GET_CONNECTOR_CAPABILITY DATA
 */
struct connector_capability_data {
    u8 OperationMode       : 8;
#define OM_DFP_ONLY         (1<<0)
#define OM_UFP_ONLY         (1<<1)
#define OM_DRP              (1<<2)
#define OM_AUDIO_SUP        (1<<3)
#define OM_DEBUG_ACC_SUP    (1<<4)
#define OM_USB2_SUP         (1<<5)
#define OM_USB3_SUP         (1<<6)
#define OM_ALT_SUP          (1<<7)
    u8 Provider            : 1;
    u8 Consumer            : 1;
    u8 Reserved            : 6;    
} ANXPACK;

#define OP_DFP                      1
#define OP_UFP                      2
#define OP_DRP                      4
#define OP_DRP_DFP                  5
#define OP_DRP_UFP                  6
/*
 * SET_UOM Command
 */
struct set_uom {
    struct control_header header;
    u16 ConnectorNumber             : 7;
    u16 USBOperationMode            : 3;
    u16 Reserved1                   : 6;
    u32 Reserved2;
} ANXPACK;

/*
 * SET_UOR Command
 */
struct set_uor {
    struct control_header header;
    u16 ConnectorNumber             : 7;
    u16 USBOperationMode            : 3;
    u16 Reserved1                   : 6;
    u32 Reserved2;
} ANXPACK;

#define PW_PROVIDER                 1
#define PW_CONSUMER                 2
#define PW_DRP                      4
#define PW_DRP_PROVIDER             5
#define PW_DRP_CONSUMER             6
/*
 * SET_PDM Command
 */
struct set_pdm {
    struct control_header header;
    u16 ConnectorNumber             : 7;
    u16 PowerDirectionMode          : 3;
    u16 Reserved1                   : 6;
    u32 Reserved2;
} ANXPACK;

/*
 * SET_PDR Command
 */
struct set_pdr {
    struct control_header header;
    u16 ConnectorNumber             : 7;
    u16 PowerDirectionMode          : 3;
    u16 Reserved1                   : 6;
    u32 Reserved2;
} ANXPACK;
#define UCSI_PDR_WAITING_TIME       75   //750ms
#define UCSI_UOR_WAITING_TIME       50   //500ms
#define UCSI_CONNECTOR_REPORT_TIME  30   //300ms

/*
 * GET_ALTERNATE_MODES Command
 */
struct get_alternate_modes {
    struct control_header header;
    u8 Recipient                 : 3;
#define R_CONNECTOR                0
#define R_SOP1                     1
#define R_SOP2                     2
#define R_SOP3                     3
    u8 Reserved1                 : 5;
    u8 ConnectorNumber           : 7;
    u8 Reserved2                 : 1;
    u8 AlternateModeOffset       : 8;
    u8 NumofAlternateMode        : 2;
    u8 Reserved3                 : 6;
    u16 Reserved4;
} ANXPACK;

/*
 * GET_ALTERNATE_MODES DATA
 */
struct alt_modes_data {
    u16 SVID0;
    u32 MID0;
    u16 SVID1;
    u32 MID1;
} ANXPACK;

/*
 * GET_CAM_SUPPORTED Command
 */
struct get_cam_supported {
    struct control_header header;
    u8 ConnectorNumber             : 7;
    u8 Reserved1                   : 1;
    u8 Reserved2;
    u32 Reserved3;
} ANXPACK;

/*
 * GET_CURRENT_CAM Command
 */
struct get_current_cam {
    struct control_header header;
    u8 ConnectorNumber             : 7;
    u8 Reserved1                   : 1;
    u8 Reserved2;
    u32 Reserved3;
} ANXPACK;

/*
 * SET_NEW_CAM Command
 */
struct set_new_cam {
    struct control_header header;
    u8 ConnectorNumber             : 7;
    u8 EnterOrExit                 : 1;
    u8 NewCAM;
    u32 AMSpecific;
} ANXPACK;

/*
 * GET_PDOS Command
 */
struct get_pdos {
    struct control_header header;
    u8 ConnectorNumber             : 7;
    u8 PartnerPDO                  : 1;
    u8 PDOOffset;
    u8 NumberOfPDOs                : 2;
    u8 SourceOrSinkPDOs            : 1;
    u8 SourceCapabilities          : 2;
#define SCTCURRENT_SUPPORTED_SOURCE_CAPABILITIES    0
#define SCTADVERTISED_CAPABILITIES                  1
#define SCTMAXIMUM_SUPPORTED_SOURCE_CAPABILITIES    2
#define SCTNOT_USED                                 3
    u8 Reserved1                   : 3;
    u8 Reserved2;
    u16 Reserved3;
} ANXPACK;

/*
 * PDOS MESSAGE
 */
struct pdos_data {
    u32 PDO[4];
} ANXPACK;

/*
 * GET_CABLE_PROPERTY Command
 */
struct get_cable_property {
    struct control_header header;
    u8 ConnectorNumber             : 7;
    u8 Reserved1                   : 1;
    u8 Reserved2;
    u32 Reserved3;
} ANXPACK;

/*
 * CABLE_PROPERTY DATA
 */
struct cable_property_data {
    u16 bmSpeedSupported;
#define SPEED_b_PER_SECOND  0
#define SPEED_Kb_PER_SECOND 1
#define SPEED_Mb_PER_SECOND 2
#define SPEED_Gb_PER_SECOND 3
    u8 bCurrentCapability;
    u8 VBUSInCable                 : 1;
    u8 CableType                   : 1;
    u8 Directionality              : 1;
    u8 PlugEndType                 : 2;
#define PLUG_TYPEA      0
#define PLUG_TYPEB      1
#define PLUG_TYPEC      2
#define PLUG_OTHER      3 //Not USB
    u8 ModeSupport                 : 1;
    u8 Reserved1                   : 2;
    u8 Latency                     : 4;
    u8 Reserved2                   : 4;
} ANXPACK;

/*
 * GET_CONNECTOR_STATUS Command
 */
struct get_connector_status {
    struct control_header header;
    u8 ConnectorNumber             : 7;
    u8 Reserved1                   : 1;
    u8 Reserved2;
    u32 Reserved3;
} ANXPACK;

/*
 * ConnectorStatusChange Field Description
 */
struct connector_status_change {
    u8 reserved1               : 1;
    u8 ExternalSupply          : 1;
    u8 PowerOperationMode      : 1;
    u8 reserved2               : 2;
    u8 SupportedProviderCap    : 1;
    u8 NegotiatedPowerLevel    : 1;
    u8 PDResetComplete         : 1;
    u8 SupportedCAM            : 1;
    u8 BatteryChargingStatus   : 1;
    u8 Reserved3               : 1;
    u8 ConnectorPartner        : 1;
    u8 PowerDirection          : 1;
    u8 Reserved4               : 1;
    u8 Connect                 : 1;
    u8 Error                   : 1;
} ANXPACK;

/*
 * CONNECTOR_STATUS DATA
 */
struct connector_status_data {
    struct connector_status_change csc;
    u8 PowerOperationMode            : 3;
#define PO_NO_CONSUMER                  0
#define PO_USB_DEFAULT                  1
#define PO_BC                           2
#define PO_PD                           3
#define PO_TYPE_C_1_5A                  4
#define PO_TYPE_C_3A                    5
    u8 ConnectStatus                 : 1;
    u8 PowerDirection                : 1;
#define PD_CONSUMER                     0
#define PD_PROVIDER                     1
    u8 ConnectorPartner              : 3;
    u8 ConnectorPartnerReserved      : 5;
#define CP_USB                          1
#define CP_ALT_MODE                     2
    u8 ConnectorPartnerType          : 3;
#define CPT_RESERVED                    0
#define CPT_DFP_ATTACHED                1
#define CPT_UFP_ATTACHED                2
#define CPT_CAB_NO_UFP_ATTACHED         3
#define CPT_CAB_UFP_ATTACHED            4
#define CPT_CAB_DEBUG_ACC               5
#define CPT_AUDIO_ACC                   6
    u32 RequestDataObject;
    u8  BatteryChargingStatus         : 2;
#define BCS_NOT_CHARGING                0
#define BCS_NOMINAL_CHARGING            1
#define BCS_SLOW_CHARGING               2
#define BCS_TRICKLE_CHARGING            3
    u8  ProviderCapLimited_Reason     : 4;
#define PCL_PowerBudgetLowered          1
#define PCL_REACH_POWER_BUDGET_LIMIT    2
    u8  Reserved                      : 2;
} ANXPACK;

/*
 * GET_ERROR_STATUS Command
 */
struct get_error_status {
    struct control_header header;
    u16 Reserved1;
    u32 Reserved2;
} ANXPACK;

/*
 * ERROR_STATUS DATA
 */
struct error_status_data {
    u16 ErrorInformation;
#define ERR_UNRECOGNIZED        (1<<0)
#define ERR_NON_EXIST           (1<<1)
#define ERR_INVALID             (1<<2)
#define ERR_INCOMPATIBLE        (1<<3)
#define ERR_CC                  (1<<4)
#define ERR_CMD_UNSUCCESS_DUE_TO_DEAD_BATTERY       (1<<5)
#define ERR_NEGOTIATION_FAIL    (1<<6)
    u8 VendorDefined[14];
} ANXPACK;
extern u8 ucsi_errno;

/*
 * UCSI state machine
 */
enum UCSI_PPM_STATE {
    PPM_IDLE_ND = 1,    //PPM IDLE, Notification disable
    PPM_IDLE_NE,        //PPM IDLE, Notification enable
    PPM_BUSY,       //PPM Busy
    PPM_PC,         //PPM Processing Command
    PPM_WCCA,       //PPM Wait for Command Completion Acknowledge
    //PPM_PCCA,     //PPM Process Command Completion Acknowledge
    //PPM_AEAC,     //PPM Asynch Event Ack Completion
    PPM_WAEA        //PPM Wait for Asynch Event Ack
};

#define S_CLEAR(X, S)   memset((void *)X, 0, sizeof(S))
#define S_CLEAR_L(X, L) memset((void *)X, 0, L)

struct ppm_ucsi_parameters {
    struct alt_modes_data alt_mode[1];
    u8 alt_mode_support_bitmap;
    u8 alt_mode_support_num;
    u8 current_alt_mode;
} ANXPACK;


/*
 * @desc   : ucsi get connectors number
 * @param  : none
 * @return : connectors number
 */

/*
 * @desc   : EC should call this function to initial UCSI framework
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 *           notify_callbak -> UCSI use this callbak function to notify BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
u8 ucsi_init();

/*
 * @desc   : ANX ucsi call this function to register platform specific function
 * @desc   : UCSI connector change notify function
 *           PPM call this after detecting USB Type-C connector status calling
 * @param  :
 *           ack : Command Completed Indicator setting
 *           connector_index : registered Connector ID
 *           wait_for_ack : Change state machine to PPM_WCCA
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
u8 ucsi_connector_change_notify_opm(u8 ack, u8 connector_index, u8 wait_for_ack);

/*
 * @desc   : UCSI PPM handler, EC should call this handler after received
 *           event from OPM/BIOS
 * @param  : none
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */

#define PD_VERSION              0x0200
#define TYPEC_VERSION           0x0110
#define UCSI_COMPLIANT_VERSION  0x0100

#define UCSI_SET_UOM_SUPPORT    (1<<0)
#define UCSI_SET_PDM_SUPPORT    (1<<1)
#define UCSI_ALT_SUPPORT        (1<<2)
#define UCSI_ALT_OR_SUPPORT     (1<<3)
#define UCSI_PDOS_SUPPORT       (1<<4)
#define UCSI_CABLE_SUPPORT      (1<<5)
#define UCSI_EXTERNAL_SUPPORT   (1<<6)
#define UCSI_PD_RESET_SUPPORT   (1<<7)

#if 1
#define ReportNegotiatedPowerLevel
#define ReportSupportedProviderCap
#define ReportPDResetComplete
#define ReportSupportedCAM
#define ReportExternalSupply

#define CMD_OPTIONAL_SUPPORT    \
        UCSI_SET_UOM_SUPPORT    | \
        UCSI_SET_PDM_SUPPORT    | \
        UCSI_ALT_SUPPORT        | \
        UCSI_ALT_OR_SUPPORT     | \
        UCSI_PDOS_SUPPORT       | \
        UCSI_CABLE_SUPPORT      | \
        UCSI_EXTERNAL_SUPPORT   | \
        UCSI_PD_RESET_SUPPORT
#else
#define CMD_OPTIONAL_SUPPORT 0x00
#endif

#if Two_PD_Chip
#define PD_MAX_INSTANCE    2
#else
#define PD_MAX_INSTANCE    1
#endif
void ucsi_async_notify_raise_up(u8 ms);
void cancel_all_timer();

/*
 * @desc   : anx_request_voltage_in_100mv, EC should call this to check the
 *           voltage while Type-C connector work as power sink.
 * @param  : port_id -> port id number (0 or 1)
 * @return :
 *           0 : Type-C connector work as power source.
 *           other : voltage, unit is 100mv.
 */
u8 anx_request_voltage_in_100mv(u8 port_id);
/*
 * @desc   : anx_request_current_in_50ma, EC should call this to check the
 *           current while Type-C connector work as power sink.
 * @param  : port_id -> port id number (0 or 1)
 * @return :
 *           0 : Type-C connector work as power source.
 *           other : current, unit is 50ma.
 */
u8 anx_request_current_in_50ma(u8 port_id);
/*
 * @desc   : anx_connector_power_status, EC should call this to check whether
 *           the PD chip is power on or power off
 * @param  : port_id -> port id number (0 or 1)
 * @return :
 *           0 : power off
 *           other : power on
 */
u8 anx_connector_power_status(u8 port_id);

void anx_ucsi_command_received(void);

void anx_ucsi_hpd_event(u8 port_id, u8 hpd);
void anx_ucsi_pin_assignment_event(u8 port_id, u8 pin_assignment);
void anx_ucsi_cc_status_event(u8 port_id, u8 cc_status);
void anx_ucsi_detach_event(u8 port_id);

#endif

