/******************************************************************************
Copyright (c) 2017, Analogix Semiconductor, Inc.

PKG Ver  : V1.1.1
Filename : anx7447_lib.h
Project  : EC UCSI Multiport
Created  : 17 Jul 2017
Devices  : ANX7447
Toolchain: C51

Description:
Define ANX7447 chip data structure, macro and interface.

******************************************************************************/

#ifndef __ANX_UCSI_H__
#define __ANX_UCSI_H__

#include <ucsi.h>


#define min(A, B) (A>B?B:A)
#define max(A, B) (A>B?A:B)

#define PDO_TYPE_FIXED       ((u32)0 << 30)
#define PDO_TYPE_BATTERY     ((u32)1 << 30)
#define PDO_TYPE_VARIABLE    ((u32)2 << 30)
#define PDO_TYPE_MASK        ((u32)3 << 30)
#define PDO_FIXED_DUAL_ROLE  ((u32)1 << 29)  /* Dual role device */
#define PDO_FIXED_SUSPEND    ((u32)1 << 28)    /* USB Suspend supported */
#define PDO_FIXED_EXTERNAL   ((u32)1 << 27)   /* Externally powered */
#define PDO_FIXED_COMM_CAP   ((u32)1 << 26)   /* USB Communications Capable */
#define PDO_FIXED_DATA_SWAP  ((u32)1 << 25)  /* Data role swap command */
#define PDO_FIXED_PEAK_CURR  ((u32)1 << 20)  /* [21..20] Peak current */
#define PDO_FIXED_HIGH_BYTE_DATA_SWAP ((u8)1 << 1)
#define PDO_FIXED_HIGH_BYTE_DUAL_ROLE ((u8)1 << 5)
/* Voltage in 50mV units */
#define PDO_FIXED_VOLT(mv) (u32)((((u32)mv)/50) << 10)
/* Max current in 10mA units */
#define PDO_FIXED_CURR(ma) (u32)((((u32)ma)/10))

/*build a fixed PDO packet*/
#define PDO_FIXED(mv, ma, flags) \
    (PDO_FIXED_VOLT(mv)\
    | PDO_FIXED_CURR(ma)\
    | (flags))

/*Pos in Data Object, the first index number begin from 0 */
#define PDO_INDEX(n, dat)     (dat << (n * PD_ONE_DATA_OBJECT_SIZE*sizeof(u8)))
#define PDO_VAR_MAX_VOLT(mv)  ((u32)((((u32)mv) / 50) & 0x3FF) << 20)
#define PDO_VAR_MIN_VOLT(mv)  ((u32)(((u32)(mv) / 50) & 0x3FF) << 10)
#define PDO_VAR_OP_CURR(ma)   ((u32)(((u32)(ma) / 10) & 0x3FF) << 0)

#define PDO_VAR(min_mv, max_mv, op_ma) \
    (PDO_VAR_MIN_VOLT(min_mv) | PDO_VAR_MAX_VOLT(max_mv) | \
    PDO_VAR_OP_CURR(op_ma) | PDO_TYPE_VARIABLE)
#define PDO_BATT_MAX_VOLT(mv) ((((mv) / 50) & 0x3FF) << 20)
#define PDO_BATT_MIN_VOLT(mv) ((((mv) / 50) & 0x3FF) << 10)
#define PDO_BATT_OP_POWER(mw) ((((mw) / 250) & 0x3FF) << 0)
#define PDO_BATT(min_mv, max_mv, op_mw) \
    (PDO_BATT_MIN_VOLT(min_mv)\
    | PDO_BATT_MAX_VOLT(max_mv)\
    | PDO_BATT_OP_POWER(op_mw)\
    | PDO_TYPE_BATTERY)

#define GET_PDO_TYPE(PDO)            ((PDO & PDO_TYPE_MASK) >> 30)
#define GET_PDO_FIXED_DUAL_ROLE(PDO) ((PDO & PDO_FIXED_DUAL_ROLE) >> 29)
#define GET_PDO_FIXED_SUSPEND(PDO)   ((PDO & PDO_FIXED_SUSPEND) >> 28)
#define GET_PDO_FIXED_EXTERNAL(PDO)  ((PDO & PDO_FIXED_EXTERNAL) >> 27)
#define GET_PDO_FIXED_COMM_CAP(PDO)  ((PDO & PDO_FIXED_COMM_CAP) >> 26)
#define GET_PDO_FIXED_DATA_SWAP(PDO) ((PDO & PDO_FIXED_DATA_SWAP) >> 25)
#define GET_PDO_FIXED_PEAK_CURR(PDO) ((PDO >> 20) & 0x03)

#define GET_PDO_FIXED_VOLT(PDO)      (((PDO >> 10) & 0x3FF) * 50)
#define GET_PDO_FIXED_CURR(PDO)      ((PDO & 0x3FF) * 10)
#define GET_VAR_MAX_VOLT(PDO)        (((PDO >> 20) & 0x3FF) * 50)
#define GET_VAR_MIN_VOLT(PDO)        (((PDO >> 10) & 0x3FF) * 50)
#define GET_VAR_MAX_CURR(PDO)        ((PDO & 0x3FF) * 10)
#define GET_BATT_MAX_VOLT(PDO)       (((PDO >> 20) & 0x3FF) * 50)
#define GET_BATT_MIN_VOLT(PDO)       (((PDO >> 10) & 0x3FF) * 50)
#define GET_BATT_OP_POWER(PDO)       (((PDO) & 0x3FF) * 250)

#define ANX_UCSI_VERSION_MAJOR       0x1
#define ANX_UCSI_VERSION_MINOR       0x1
#define ANX_UCSI_VERSION_REV         0x1

#define POWER_ROLE_SWAP_DELAY_MS     50
#define DATA_ROLE_SWAP_DELAY_MS      50

#define PD_ONE_DATA_OBJECT_SIZE      4
#define PD_MAX_DATA_OBJECT_NUM       7
#define VDO_SIZE                     (PD_ONE_DATA_OBJECT_SIZE * PD_MAX_DATA_OBJECT_NUM)

#define PDO_FIXED_FLAGS              (PDO_FIXED_DUAL_ROLE | PDO_FIXED_DATA_SWAP)
/*5000mv voltage*/
#define PD_VOLTAGE_5V         5000
#define PD_MAX_VOLTAGE_20V    20000
/*0.9A current */
#define PD_CURRENT_900MA      900
#define PD_CURRENT_1500MA     1500
#define PD_CURRENT_3A         3000
#define PD_CURRENT_2A         2000
#define PD_POWER_15W          15000
#define PD_POWER_40W          40000
#define PD_POWER_60W          60000

/* RDO : Request Data Object */
#define RDO_OBJ_POS(n)             (((u32)(n) & 0x7) << 28)
#define RDO_POS(rdo)               ((((32)rdo) >> 28) & 0x7)
#define RDO_GIVE_BACK              ((u32)1 << 27)
#define RDO_CAP_MISMATCH           ((u32)1 << 26)
#define RDO_COMM_CAP               ((u32)1 << 25)
#define RDO_NO_SUSPEND             ((u32)1 << 24)
#define RDO_FIXED_VAR_OP_CURR(ma)  (((((u32)ma) / 10) & 0x3FF) << 10)
#define RDO_FIXED_VAR_MAX_CURR(ma) (((((u32)ma) / 10) & 0x3FF) << 0)

#define RDO_BATT_OP_POWER(mw)      (((((u32)mw) / 250) & 0x3FF) << 10)
#define RDO_BATT_MAX_POWER(mw)     (((((u32)mw) / 250) & 0x3FF) << 10)

#define RDO_FIXED(n, op_ma, max_ma, flags)      \
        (RDO_OBJ_POS(n) | (flags) |             \
        RDO_FIXED_VAR_OP_CURR(op_ma) |          \
        RDO_FIXED_VAR_MAX_CURR(max_ma))

#define RDO_BATT(n, op_mw, max_mw, flags) \
			(RDO_OBJ_POS(n) | (flags)|\
			RDO_BATT_OP_POWER(op_mw)|\
			RDO_BATT_MAX_POWER(max_mw))
#define EXTERNALLY_POWERED  YES
/* Source Capabilities */
/* 1 to 5 */
#define SOURCE_PROFILE_NUMBER   1
/* 0 = Fixed, 1 = Battery, 2 = Variable */
#define SRC_PDO_SUPPLY_TYPE1    0
/* 0 to 3 */
#define SRC_PDO_PEAK_CURRENT1   0
/* 5000mV (5V) */
#define SRC_PDO_VOLTAGE1        5000
/* 500mA (0.5A) */
#define SRC_PDO_MAX_CURRENT1    500

#define YES     1
#define NO      0
#define ERR_CABLE_UNPLUG -1

#define MV 1
#define MA 1
#define MW 1

typedef enum {
    TYPE_PWR_SRC_CAP = 0x00,
    TYPE_PWR_SNK_CAP = 0x01,
    TYPE_DP_SNK_IDENTITY = 0x02,
    TYPE_SVID = 0x03,
    TYPE_GET_DP_SNK_CAP = 0x04,
    TYPE_ACCEPT = 0x05,
    TYPE_REJECT  = 0x06,
    TYPE_DP_SNK_STATUS = 0x07,
    TYPE_SET_SNK_DP_CAP = 0x08,
    TYPE_GET_SRC_CAP = 0x09,
    TYPE_GET_RDO = 0x0a,

    TYPE_PSWAP_REQ = 0x10,
    TYPE_DSWAP_REQ = 0x11,
    TYPE_GOTO_MIN_REQ =  0x12,
    TYPE_VCONN_SWAP_REQ = 0x13,
    TYPE_VDM = 0x14,
    TYPE_DP_SNK_CFG = 0x15,
    TYPE_PWR_OBJ_REQ = 0x16,
    TYPE_GET_PD_STATUS = 0x17,
    TYPE_DP_ALT_ENTER = 0x19, 
    TYPE_DP_ALT_EXIT = 0x1A, 
    TYPE_GET_SNK_CAP = 0x1B, 
    TYPE_NOT_SUPPORTED = 0x1C, // PD_CTRL_NOT_SUPPORTED
    TYPE_GET_SRC_CAP_EXT = 0x1D, // PD_CTRL_GET_SOURCE_CAP_EXTENDED
    TYPE_GET_STS = 0x1E, // PD_CTRL_GET_STATUS
    TYPE_FR_SWAP = 0x1F, // PD_CTRL_FR_SWAP
    TYPE_FR_SWAP_SIGNAL = 0x20, // Fast Role Swap signal
    TYPE_GET_PPS_STS = 0x21, //PD_CTRL_GET_PPS_STS
    TYPE_GET_COUNTRY_CODE = 0x22, //PD_CTRL_GET_COUNTRY_CODE
    TYPE_GET_SINK_CAP_EXT = 0x23, //PD_CTRL_GET_SINK_CAP_EXTENDED
    TYPE_BATT_STS = 0x24, // PD_DATA_BATTERY_STATUS
    TYPE_ALERT = 0x25, // PD_DATA_ALERT
    TYPE_GET_COUNTRY_INFO = 0x26, //PD_DATA_GET_COUNTRY_INFO
    TYPE_DP_DISCOVER_MODES_INFO = 0x27, //PD DISCOVERY MODE
    TYPE_GET_DP_CONFIGURE   = 0x29,// for Pin Assignment
    TYPE_DP_CONFIGURE   = 0x2a,// for Pin Assignment
    TYPE_AMS_PROCESSING = 0x2b, // Processing AMS, Collision Avoidance
    TYPE_CMD_SRC_CAP_1 = 0x2c, // Update source capability to OCM without sending to CC line

    TYPE_EXT_SRC_CAP_EXT = 0x30, // Source_Capabilities_Extended
    TYPE_EXT_STS = 0x31, // Status
    TYPE_EXT_GET_BATT_CAP = 0x32, // Get_Battery_Cap
    TYPE_EXT_GET_BATT_STS = 0x33, // Get_Battery_ Status
    TYPE_EXT_BATT_CAP = 0x34, // Battery_Capabilities
    TYPE_EXT_GET_MFR_INFO = 0x35, // Get_Manufacturer_Info
    TYPE_EXT_MFR_INFO = 0x36, // Manufacturer_Info
    TYPE_EXT_PPS_STS  = 0x37, //PPS_status
    TYPE_EXT_COUNTRY_INFO  = 0x38, // Country_info
    TYPE_EXT_COUNTRY_CODE  = 0x39, // Country_code
    TYPE_EXT_SINK_CAP_EXT = 0x3a, // SINK_Capabilities_Extended
    TYPE_EXT_PDFU_REQUEST = 0x3b,
    TYPE_EXT_PDFU_RESPONSE = 0x3c,
    TYPE_I2C_MASTER_WRITE = 0x3e, //I2C Master write
    
    TYPE_RESPONSE_TO_REQ = 0xF0,
    TYPE_SOFT_RST = 0xF1,
    TYPE_HARD_RST = 0xF2,
    TYPE_RESTART = 0xF3,
    TYPE_GET_VAR = 0xFC, // get variable value
    TYPE_SET_VAR = 0xFD // set variable value
} PD_MSG_TYPE;

typedef enum {// TRANSMIT_SOP_TYPE
    SOP_TYPE  = 0x00, 
    SOP1_TYPE = 0x01, 
    SOP2_TYPE = 0x02, 
} PD_SOP_TYPE;

struct cable_vdo {
    u8 hardware_version      : 4;
    u8 firmware_version      : 4;
    u16 reserved             : 4;
    u16 cable_type           : 2;
    u16 type_c_plug_to_p_r   : 1;
    u16 latency              : 4;
    u16 termination_type     : 2;
    u16 SSTX1_sup            : 1;
    u16 SSTX2_sup            : 1;
    u16 SSRX1_sup            : 1;
    u8 SSRX2_sup             : 1;
    u8 vbus_current          : 2;
#define VBUS_NOT_THRU_CABLE    0
#define VBUS_3A                1
#define VBUS_5A                2
    u8 vbus_thru_cable       : 1;
    u8 sop2_present          : 1;
    u8 supper_speed_signal   : 3;
#define SUPPER_SPEED_USB_2_0    0
#define SUPPER_SPEED_USB_3_1    1
#define SUPPER_SPEED_USB_3_1_2  2
} ANXPACK;

typedef enum {
    // for TYPE_GET_VAR and TYPE_SET_VAR
    // use correct offset and length for each variable type
    // do not use no define variable type
    IF_VAR_fw_var_reg = 0x10, // offset:0, length:8
    IF_VAR_pd_src_pdo = 0x11, // offset:0, length:28 (VDO_SIZE)
    IF_VAR_pd_snk_pdo = 0x12, // offset:0, length:28 (VDO_SIZE)
    IF_VAR_pd_rdo_bak = 0x13, // offset:0, length:4 (VDO_LEN)
    IF_VAR_pd_rdo = 0x14, // offset:0, length:4 (VDO_LEN)
    IF_VAR_DP_caps = 0x15, // offset:0, length:4 (VDO_LEN)
    IF_VAR_configure_DP_caps = 0x16, // offset:0, length:4 (VDO_LEN)
    IF_VAR_src_dp_status = 0x17, // offset:0, length:4 (VDO_LEN)
    IF_VAR_sink_svid_vdo = 0x18, // offset:0, length:4 (VDO_LEN)
    IF_VAR_sink_identity = 0x19, // offset:0, length:16
}PD_MSG_VAR_TYPE;

typedef enum {
    AttachWait_SNK = 0x00,
    AttachWait_SRC = 0x01,
    TRY_SNK= 0x02,
    TRY_SRC = 0x03,
    Attached_SNK  = 0x04,
    Attached_SRC = 0x05,
    Unattached_SNK  = 0x06,
    Unattached_SRC = 0x07,
    TRY_SNK_WaitVbus=0x08,
    TRY_SNK_WaitRp=0x09,
    TRY_SRC_WaitRd=0X0A,
    Error_SNK=0x0B
}TYPEC_STATE;

typedef enum {
    CABLE_TYPE_A = 1,
    CABLE_TYPE_B,
    CABLE_TYPE_C,
} cable_type;

/*Comands status*/
enum interface_status {
    CMD_SUCCESS,
    CMD_REJECT,
    CMD_FAIL,
    CMD_BUSY,
    CMD_STATUS
};

struct anx_cam {
    u16 svid;
    u32 mid;
} ANXPACK;

struct anx_context {
    /* initial data */
    //u8 init_svid[4];
    //u8 init_snk_ident[16];
    //u32 init_snk_cap[3];
    //u32 init_src_caps[1];

    u8 anx_power_status;
	u8 anx_power_role;
	u8 anx_data_role;
	u8 anx_current_in_50ma;
	u8 anx_voltage_in_100mv;
    u8 pd_snk_pdo_cnt;
    u8 pd_snk_pdo[VDO_SIZE];
    u8 pd_rdo[PD_ONE_DATA_OBJECT_SIZE];
    u8 DP_caps[PD_ONE_DATA_OBJECT_SIZE];
    u8 configure_DP_caps[PD_ONE_DATA_OBJECT_SIZE];
    u8 src_dp_caps[PD_ONE_DATA_OBJECT_SIZE];
    u8 downstream_pd_cap;
    struct anx_cam ac[2];
    struct cable_vdo cv;
    u8 peer_svid[16];
    u8 peer_svid_length;
    u8 pd_src_pdo_cnt;
    u8 pd_src_pdo[VDO_SIZE];

    cable_type ct;
    u8 ucsi_connector_partner_type;
    u8 ConnectorPartner;
    u8 BatteryCharging;
    u32 ucsi_partner_rdo;
    u8 power_op_mode;
    u8 power_role_swap_flag;
    u8 partner_pdo_length;
    u8 dfp_only;
    u8 partner_pdo_sink_or_source;
    u8 partner_pdo[VDO_SIZE];
    u8 sel_voltage_pdo_index;
    u8 sys_sta_bak;
    //u8 power_operation_mode;
    u8 enable_report_power_on;
    u8 data_operation_mode_uom;
    u8 power_operation_mode_pdm;
    u8 data_operation_mode_uor;
    u8 power_operation_mode_pdr;
    u8 hpd_status;

    //struct connector_status_change anx_csc;
    union {
        u16 connector_change_indicator;
        struct connector_status_change csc;
    }csc;
    u8 anx_vbus_status;
    u8 anx_initial_status;
#define INIT_STAGE_1    1
#define INIT_STAGE_2    2
#define INIT_STAGE_3    3
#define INIT_STAGE_4    4
#define INIT_STAGE_5    5
#define INIT_DONE       0

    /* buffer pointer */
    u8 pbuf_rx_front;
    u8 pbuf_tx_rear;

    u8 *InterfaceRecvBuf;
    u8 *InterfaceSendBuf;

    u8 connector_index;

    u8 trysnk_enable;
    u8 trysrc_enable;
    u8 pd_capability;
#define PD_HAS_RETURN_STATUS    (1<<0)
#define PD_HAS_SVID_MID         (1<<1)
#define PD_HAS_CABLE_VDO        (1<<2)
#define PD_HAS_REDRIVER         (1<<3)

    /* I2C Slave address */
    u8 PD_standalone_slave_id1;
    u8 PD_standalone_slave_id2;
#define addr    PD_standalone_slave_id1
#define addr2   PD_standalone_slave_id2
    u8 PD_EMTB_slave_id;
    u8 PD_EMRB_slave_id;
    u8 PD_SPI_slave_id;
#define spi PD_SPI_slave_id
    u8 PD_TCPM_slave_id;

    /* GPIO definition */
    //u8 cable_detect_gpio;
    //u8 power_en_gpio;
    //u8 reset_gpio;
    //u8 alert_gpio;
    u8 mux_mode;
    u8 cc_orientation;
    u8 error_count;
} ANXPACK;
//redriver control
#define ANXREDRIVER
#define NO_CONNECT              0x00
#define USB3_1_CONNECTED        0x01
#define DP_ALT_4LANES           0x02
#define USB3_1_DP_2LANES        0x03
#define CC1_CONNECTED           0x00
#define CC2_CONNECTED           0x01
#define SELECT_PIN_ASSIGMENT_C  0x04
#define SELECT_PIN_ASSIGMENT_D  0x08
#define SELECT_PIN_ASSIGMENT_E  0x10
#define SELECT_PIN_ASSIGMENT_U  0x00
#define REDRIVER_ADDRESS        0x20
#define REDRIVER_OFFSET         0x00

//#define ANX_I2C_BUFF_FAST_READ
//#define ANX_POWER_ON_DELAY_ISSUE
#ifdef ANX_POWER_ON_DELAY_ISSUE
#define ANX_SEND_GET_SOURCE_CAPS
#define ANX_OVP_RE_SET
#endif

s8 *result_to_str(u8 result_type);
s8 *interface_to_str(u8 header_type);
void anx_initial_context(u8 port_index);
u8 send_pd_msg(PD_MSG_TYPE type,
        u8 *buf, u8 size, u8 type_sop);
u8 get_data_role();
u8 get_power_role();
void anx7447_ucsi_init();
void send_initialized_setting();
void anx_alert_message_isr();
void dp_alt_exit_mode();
void dp_alt_enter_mode();
void anx_dead_battery_checking_1();
void send_source_capability(void);
void send_source_sink_capability(void);
void chip_power_on(u8 port);
void chip_power_down(u8 port);
#endif
