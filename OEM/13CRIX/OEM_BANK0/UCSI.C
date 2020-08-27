/******************************************************************************
Copyright (c) 2017, Analogix Semiconductor, Inc.

PKG Ver  : V1.1.1
Filename : ucsi.c
Project  : EC UCSI Multiport
Created  : 17 Jul 2017
Devices  : ANX7447
Toolchain: C51

Description:
UCSI public layer.
The basic UCSI COMMANDs public interface implementation.

Revision History:
V0.1
1. Initial version, implemented the basic required COMMANDs.

V0.2
1. Add connector exist checking for command GET_ALTERNATE_MODES

v0.6.0
1. Used 1ms hook and 50ms hook to check Async callback, event, GPIO interrupt.
2. Used GPIO Port H.6 as Liberty Type-C port1 interrupt pin, configured as "INPUT", active is low.
3. Used GPIO Port H.5 as Liberty Type-C port0 interrupt pin, configured as "INPUT", active is low.
4. UCSI notify OPM by calling interface "ucsi_ppm_notify_opm"
5. Once EC received event from OPM, EC should copy all related data to data structure "ppm2bios" which has been defined in ucsi.c.
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
2. Shrink SET_PDR waiting time to 750ms, JIRA: LBT-412.

v0.8.4
1. Fix several USB Type-C PD 2.0 CTS failed items, LBT-413.
2. Fix initialization message(such as SVID, DP_CAPS) didn't send to OCM on time issue, LBT-414.

v1.0.1
Based on v0.8.4.

v1.0.2
Fix JIRA LBT-366, LBT-404, LBT-429

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
1.Enable SET_UOM feature, fix HLK failed item, JIRA LBT-552

v1.1.1
1. Change the version v1.0.c to v1.1.1.
******************************************************************************/
#include <stdio.h>
#include <string.h>

#include <UCSI.H>
#include <ANX7447_LIB.H>

#define UCSI_baseaddr   0x0A00
static enum UCSI_PPM_STATE ANXXDATA ucsi_ppm_state                 _at_ UCSI_baseaddr + 0x0;
u8 ANXXDATA ucsi_errno                                             _at_ UCSI_baseaddr + 0x1;
u8 ANXXDATA self_charging_error                                    _at_ UCSI_baseaddr + 0x5;
static struct error_status_data ANXXDATA esd                       _at_ UCSI_baseaddr + 0x6;
static u8 ANXXDATA connector_change_need_ack                       _at_ UCSI_baseaddr + 0x16;
static u8 ANXXDATA error_need_ack                                  _at_ UCSI_baseaddr + 0x17;
static u8 ANXXDATA command_complete_need_ack                       _at_ UCSI_baseaddr + 0x18;
static u8 ANXXDATA need_ack_command                                _at_ UCSI_baseaddr + 0x19;
u8 ANXXDATA random_data                                            _at_ UCSI_baseaddr + 0x1A;
static u8 ANXXDATA registered_connector_index                      _at_ UCSI_baseaddr + 0x1B;
u8 ANXXDATA timer1msCount                                          _at_ UCSI_baseaddr + 0x1c;
u8 ANXXDATA i                                                      _at_ UCSI_baseaddr + 0x1F;
u8 ANXXDATA j                                                      _at_ UCSI_baseaddr + 0x20;
u8 ANXXDATA k                                                      _at_ UCSI_baseaddr + 0x21;
u8 ANXXDATA l                                                      _at_ UCSI_baseaddr + 0x22;
u8 ANXXDATA DataLength                                             _at_ UCSI_baseaddr + 0x23;
u8 ANXXDATA wait_for_ack_back                                      _at_ UCSI_baseaddr + 0x24;
/* reserved 0x25 ~ 0x27 */
u8 current_pending_command_timeout                                 _at_ UCSI_baseaddr + 0x28;
u16 ucsi_async_checking_timer                                      _at_ UCSI_baseaddr + 0x29;
u8 ucsi_async_restore                                              _at_ UCSI_baseaddr + 0x2B;
u8 ucsi_command_process_delay                                      _at_ UCSI_baseaddr + 0x2C;
struct notification_enable_status ANXXDATA ne                      _at_ UCSI_baseaddr + 0x2F;
/* reserved 0x38 ~ 0x76 */
u8 ANXXDATA result                                                 _at_ UCSI_baseaddr + 0x77;
u8 ANXXDATA port_index                                             _at_ UCSI_baseaddr + 0x78;
u8 ANXXDATA ucsi_received_command                                  _at_ UCSI_baseaddr + 0x79;
u8 ANXXDATA role                                                   _at_ UCSI_baseaddr + 0x7a;
u8 ANXXDATA c                                                      _at_ UCSI_baseaddr + 0x7b;
u8 ANXXDATA len                                                    _at_ UCSI_baseaddr + 0x7c;
u16 ANXXDATA count                                                 _at_ UCSI_baseaddr + 0x7d;
u8 ANXXDATA port_id                                                _at_ UCSI_baseaddr + 0x7f;

#define INTERFACE_SEND_BUF_SIZE 32
#define INTERFACE_RECV_BUF_SIZE 32

u8 InterfaceSendBuf[PD_MAX_INSTANCE][INTERFACE_SEND_BUF_SIZE]      _at_ UCSI_baseaddr + 0x80;
u8 InterfaceRecvBuf[PD_MAX_INSTANCE][INTERFACE_RECV_BUF_SIZE]      _at_ UCSI_baseaddr + 0xc0;
struct ppm_ucsi_parameters ANXXDATA pup                            _at_ UCSI_baseaddr + 0x1c0;

//TF_006--	#define ANXLIB_baseaddr     0x0B00
#define ANXLIB_baseaddr     0x0C00	//TF_006++	
struct anx_context ANXXDATA context[PD_MAX_INSTANCE]               _at_ ANXLIB_baseaddr + 0x0;
struct bios_share_msg ANXXDATA ppm2bios                            _at_ ANXLIB_baseaddr + 0x1b2;
u8 ANXXDATA SWAP_BYTE                                              _at_ ANXLIB_baseaddr + 0x1e2;
u8 ANXXDATA supply_1500ma_flag                                     _at_ ANXLIB_baseaddr + 0x1e3;
u8 ANXXDATA supply_1500ma_port                                     _at_ ANXLIB_baseaddr + 0x1e4;
u8 ANXXDATA request_src_caps_flag[2]                               _at_ ANXLIB_baseaddr + 0x1e5;
u8 ANXXDATA change_int                                             _at_ ANXLIB_baseaddr + 0x1f0;
u8 ANXXDATA change_status                                          _at_ ANXLIB_baseaddr + 0x1f1;
u8 ANXXDATA intr_alert_0                                           _at_ ANXLIB_baseaddr + 0x1f2;
u8 ANXXDATA intr_alert_1                                           _at_ ANXLIB_baseaddr + 0x1f3;
u16 ANXXDATA pdo_h                                                 _at_ ANXLIB_baseaddr + 0x1f4;
u16 ANXXDATA pdo_l                                                 _at_ ANXLIB_baseaddr + 0x1f6;
u32 ANXXDATA pdo_max                                               _at_ ANXLIB_baseaddr + 0x1f8;
u32 ANXXDATA pdo_max_tmp                                           _at_ ANXLIB_baseaddr + 0x1fc;

extern code u32 snk_cap[];
#define current_pending_command (ch->Command)

#define msg    (&ppm2bios)
#define cci    ((struct cci_status *)(&ppm2bios.CCI))
#define sne    ((struct set_notification_enable *)(&ppm2bios.CONTROL))
#define ch     ((struct control_header *)(&ppm2bios.CONTROL))
#define cr     ((struct connector_reset *)(&ppm2bios.CONTROL))
#define acc    ((struct ack_cc_ci*)(&ppm2bios.CONTROL))
#define gcc    ((struct get_connector_capability *)(&ppm2bios.CONTROL))
#define suor   ((struct set_uor *)(&ppm2bios.CONTROL))
#define spdr   ((struct set_pdr *)(&ppm2bios.CONTROL))
#define suom   ((struct set_uom *)(&ppm2bios.CONTROL))
#define spdm   ((struct set_pdm *)(&ppm2bios.CONTROL))
#define gcs    ((struct get_cam_supported *)(&ppm2bios.CONTROL))
#define gcp    ((struct get_cable_property *)(&ppm2bios.CONTROL))
#define gp     ((struct get_pdos *)(&ppm2bios.CONTROL))
#define snc    ((struct set_new_cam *)(&ppm2bios.CONTROL))
#define gam    ((struct get_alternate_modes *)(&ppm2bios.CONTROL))
#define csd    ((struct connector_status_data *)(&ppm2bios.msgi))
#define amd    ((struct alt_modes_data *)(&ppm2bios.msgi))
#define pd     ((struct pdos_data *)(&ppm2bios.msgi))
#define cpd    ((struct cable_property_data *)(&ppm2bios.msgi))
#define cd     ((struct capability_data *)(&ppm2bios.msgi))
#define ccd    ((struct connector_capability_data *)(&ppm2bios.msgi))


#define DETECT_POWER_ON(port_index) \
    if (context[port_index].anx_power_status == 0) { \
        ucsi_errno = ERR_INVALID; \
        set_error_data(); \
        return UCSI_COMMAND_FAIL; \
    }

#ifdef ANXDEBUG
#define SET_STATE(X)    {\
    ucsi_debug("%s:%d change state(%d) to state(%d).\n", \
            __func__, __LINE__, \
            (u16)ucsi_ppm_state, (u16)X); \
    ucsi_ppm_state = X; \
}
#else
#define SET_STATE(X)    {\
    ucsi_ppm_state = X; \
}

#endif

#define GOTO_STATE(X)   SET_STATE(X)

/*
 * @desc   : ucsi detect connector change
 * @param  :
 *           cci -> cci status structure
 * @return : void
 */
void ucsi_ppm_connector_change_detect()
{
    /* Connector change notification is disabled */
    if (ne.ConnectChange == 0)
        return;

    for (i = 0; i < PD_MAX_INSTANCE; i++)
        if ((context[i].csc.connector_change_indicator) != 0) {
            cci->ConnectorChangeIndicator = i + 1;
            break;
        }
}

/*
 * @desc   : SET_NOTIFICATION_ENABLE ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void set_notification_enable_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC)
        cci->ErrorIndicator = 0;
    else
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : CANCEL ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static u8 cancel_ack()
{
    S_CLEAR(cci, struct cci_status);
    
    ucsi_ppm_connector_change_detect();

    cci->CommandCompletedIndicator = 1;

    return UCSI_COMMAND_SUCC;
}

/*
 * @desc   : COMMAND BUSY ACK setting
 * @param  :
 *           cci -> cci status structure
 * @return : void
 */
static void busy_ack()
{
    S_CLEAR(cci, struct cci_status);
    cci->BusyIndicator = 1;
    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : COMMAND ERROR ACK setting
 * @param  :
 *           cci -> cci status structure
 * @return : void
 */
static void error_ack()
{
    S_CLEAR(cci, struct cci_status);
    cci->ErrorIndicator = 1;
    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : GET_CONNECTOR_RESET ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void connector_reset_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result != UCSI_COMMAND_SUCC)
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : ACK_CC_CI ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void ack_cc_ci_ack()
{
    S_CLEAR(cci, struct cci_status);
    
    cci->AcknowledgeCommandIndicator = 1;
}

/*
 * @desc   : PPM_RESET ERROR ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void reset_ack()
{
    S_CLEAR(cci, struct cci_status);

    if (result == UCSI_COMMAND_SUCC)
        cci->ResetCompletedIndicator = 1;
}

/*
 * @desc   : GET_CAPABILITY ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void get_capability_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC)
        cci->DataLength = 0x10;
    else
        cci->ErrorIndicator = 0x1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : GET_CONNECTOR_CAPABILITY ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
*/
static void get_connector_capability_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC)
        cci->DataLength = 2;
    else
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : SET_UOR ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void set_uor_ack()
{
    S_CLEAR(cci, struct cci_status);

    //ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_FAIL)
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : SET_PDR ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void set_pdr_ack()
{
    S_CLEAR(cci, struct cci_status);

    //ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_FAIL)
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : GET_CONNECTOR_STATUS ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void get_connector_status_ack()
{
    S_CLEAR(cci, struct cci_status);

    //ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC)
        cci->DataLength = 0x09;
    else
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : GET_ERROR ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
static void get_error_status_ack()
{
    S_CLEAR(cci, struct cci_status);

    //ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC)
        //Currently, we do not support Vendor Defined Error Code
        cci->DataLength = 0x10;
    else
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}

/*
 * @desc   : GET_CABLE_PROPERTY ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
#if UCSI_CABLE_SUPPORT
static void get_cable_property_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC)
        //Currently, we do not support Vendor Defined Error Code
        cci->DataLength = 0x05;
    else
        cci->ErrorIndicator = 1;

    cci->CommandCompletedIndicator = 1;
}
#endif

/*
 * @desc   : GET_PDOS ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 *           DataLength -> message actually used byte
 * @return : void
 */
#if UCSI_PDOS_SUPPORT
static void get_pdos_ack()
{
    S_CLEAR(cci, struct cci_status);

    //ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC) {
        cci->DataLength = DataLength;
    } else {
        cci->ErrorIndicator = 1;
    }

    cci->CommandCompletedIndicator = 1;
}
#endif

/*
 * @desc   : SET_NEW_CAM ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
#if UCSI_ALT_OR_SUPPORT
static void set_new_cam_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC) {
        cci->DataLength = 0;
    } else {
        cci->ErrorIndicator = 1;
    }

    cci->CommandCompletedIndicator = 1;
}
#endif

/*
 * @desc   : GET_CURRENT_CAM ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 * @return : void
 */
#if UCSI_ALT_SUPPORT
static void get_current_cam_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC) {
        cci->DataLength = 1;
    } else {
        cci->ErrorIndicator = 1;
    }

    cci->CommandCompletedIndicator = 1;
}
#endif

/*
 * @desc   : GET_CAM_SUPPORT ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 *           DataLength -> message actually used byte
 * @return : void
 */
#if UCSI_ALT_SUPPORT
static void get_cam_supported_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC) {
        cci->DataLength = DataLength;
    } else {
        cci->ErrorIndicator = 1;
    }

    cci->CommandCompletedIndicator = 1;
}
#endif

/*
 * @desc   : GET_ALT_MODES ACK setting
 * @param  :
 *           cci -> cci status structure
 *           result -> ACK success or fail
 *           DataLength -> message actually used byte
 * @return : void
 */
#if UCSI_ALT_SUPPORT
static void get_alt_modes_ack()
{
    S_CLEAR(cci, struct cci_status);

    ucsi_ppm_connector_change_detect();

    if (result == UCSI_COMMAND_SUCC) {
        cci->DataLength = DataLength;
    } else {
        cci->ErrorIndicator = 1;
    }

    cci->CommandCompletedIndicator = 1;
}
#endif

//-----------------------------------------------------------

/*
 * @desc   : store error info in error status structure -> esd
 * @param  :
 *           cci -> cci status structure
 *           errno -> error number
 * @return : void
 */
static void set_error_data()
{

    S_CLEAR(&esd, struct error_status_data);
    esd.ErrorInformation = ucsi_errno;
    
    error_ack();
}

/*
 * @desc   : process unrecognized command
 * @param  :
 *           cci -> cci status structure
 * @return : UCSI_COMMAND_SUCC
 */
static u8 unsupport_command_process()
{
    ucsi_errno = ERR_UNRECOGNIZED;
    set_error_data();
    cci->NotSupportedIndicator = 1;
    cci->CommandCompletedIndicator = 1;

    //Here we abused to use SUCCESS as return value
    return UCSI_COMMAND_SUCC;
}

#if 0
/*
 * @desc   : checking the connector whether exist in PPM
 * @param  :
 *           cci -> cci status structure
 *           connectorId -> OPM passed
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 check_connector_exist(struct cci_status *cci, u8 connectorId)
{
    u8 ret = UCSI_COMMAND_SUCC;

#ifdef UCSI_SUPPORT_MORE_CONNECTOR
    //Add more connector checking code here.
    return ret;
#else
    if (connectorId != 1) {
        ucsi_errno = ERR_NON_EXIST;
        //set_error_data(cci, ERR_NON_EXIST);
        ret = UCSI_COMMAND_FAIL;
    }

    return ret;
#endif
}
#endif

/*
 * @desc   : OPM get alt modes from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
#if UCSI_ALT_SUPPORT
static u8 ppm_get_alt_modes()
{
    //u8 DataLength;
    //u8 i;
    //u8 Offset, Altnum;

    if (gam->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    port_index = gam->ConnectorNumber - 1;

    //Offset = gam->AlternateModeOffset;
    //Altnum = gam->NumofAlternateMode;
    
    //if (gam->Recipient != R_CONNECTOR)
    //    DETECT_POWER_ON(port_index);
    if (gam->AlternateModeOffset > pup.alt_mode_support_num) {
        //ucsi_errno = ERR_INVALID;
        //set_error_data();
        DataLength = 0;
        goto out;;
    }

    if (gam->NumofAlternateMode > (pup.alt_mode_support_num - gam->AlternateModeOffset))
        gam->NumofAlternateMode = (pup.alt_mode_support_num - gam->AlternateModeOffset);

    /* Currently, we only support connector and SOP as recipient */
    switch (gam->Recipient) {
    case R_CONNECTOR:
        for (i = 0; i< gam->NumofAlternateMode; i++) {
            amd[i] = pup.alt_mode[gam->AlternateModeOffset +i];
            #ifdef EC_ANX_BIG_ENDIAN
            SWAP_WORD(&amd[i].SVID0);
            SWAP_DWORD(&amd[i].MID0);
            SWAP_WORD(&amd[i].SVID1);
            SWAP_DWORD(&amd[i].MID1);
            #endif
        }
        DataLength = 6;
        break;
    case R_SOP1:
        DataLength = 0;
        switch (context[port_index].peer_svid_length) {
        case 4:
            pup.alt_mode_support_bitmap = 1;
            if (context[port_index].pd_capability & PD_HAS_SVID_MID) {
                amd[0].SVID0 = context[port_index].ac[0].svid;
                amd[0].MID0 = context[port_index].ac[0].mid;
                #ifdef EC_ANX_BIG_ENDIAN
                SWAP_WORD(&amd[0].SVID0);
                //SWAP_DWORD(&amd[0].MID0);
                #endif
            } else {
                amd[0].SVID0 = (context[port_index].peer_svid[3] << 8) | context[port_index].peer_svid[2];
                amd[0].MID0 = 0x405;
                #ifdef EC_ANX_BIG_ENDIAN
                SWAP_WORD(&amd[0].SVID0);
                SWAP_DWORD(&amd[0].MID0);
                #endif
            }
            DataLength = 6;
            break;
        case 8:
            pup.alt_mode_support_bitmap = 1;
            if (context[port_index].pd_capability & PD_HAS_SVID_MID) {
                amd[0].SVID0 = context[port_index].ac[0].svid;
                amd[0].MID0 = context[port_index].ac[0].mid;
                amd[0].SVID1 = context[port_index].ac[1].svid;
                amd[0].MID1 = context[port_index].ac[1].mid;
                #ifdef EC_ANX_BIG_ENDIAN
                SWAP_WORD(&amd[i].SVID0);
                //SWAP_DWORD(&amd[i].MID0);
                SWAP_WORD(&amd[i].SVID1);
                //SWAP_DWORD(&amd[i].MID1);
                #endif
            } else {
                amd[0].SVID0 = (context[port_index].peer_svid[3] << 8) | context[port_index].peer_svid[2];
                amd[0].MID0 = 0x405;
                amd[0].SVID1 = (context[port_index].peer_svid[1] << 8) | context[port_index].peer_svid[0];
                amd[0].MID1 = 0;
                #ifdef EC_ANX_BIG_ENDIAN
                SWAP_WORD(&amd[i].SVID0);
                SWAP_DWORD(&amd[i].MID0);
                SWAP_WORD(&amd[i].SVID1);
                SWAP_DWORD(&amd[i].MID1);
                #endif
            }
            DataLength = 12;
            break;
        case 0:
        default:
            DataLength = 0;
            break;
        }
        break;
    default:
        for (i = 0; i < gam->NumofAlternateMode; i++) {
            S_CLEAR(&amd[i], struct alt_modes_data);
        }
        DataLength = 0;
        break;
    }

out:
    result = UCSI_COMMAND_SUCC;
    get_alt_modes_ack();

    return result;
}
#endif

/*
 * @desc   : OPM get cam suported list from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
#if UCSI_ALT_SUPPORT
static u8 ppm_get_cam_supported()
{
    //u8 DataLength;

    if (gcs->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    if (context[gcs->ConnectorNumber].peer_svid_length == 0) {
        DataLength = 0;
        goto out;
    }

    *(char *)&msg->msgi = pup.alt_mode_support_bitmap;

    if (pup.alt_mode_support_bitmap)
        DataLength = (pup.alt_mode_support_num % 8) + 1;
    else
        DataLength = 0;

out:
    result = UCSI_COMMAND_SUCC;
    get_cam_supported_ack();

    return result;
}
#endif

/*
 * @desc   : OPM get current cam from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
#if UCSI_ALT_SUPPORT
static u8 ppm_get_current_cam()
{
    if (gcc->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    if (context[gcc->ConnectorNumber].peer_svid_length == 0) {
        *(char *)&msg->msgi = 0;
        goto out;
    }

    *(char *)&msg->msgi = 1;
out:
    result = UCSI_COMMAND_SUCC;
    get_current_cam_ack();

    return result;
}
#endif

/*
 * @desc   : OPM set new cam for PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
#if UCSI_ALT_OR_SUPPORT
static u8 ppm_set_new_cam()
{
    if (snc->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    if (context[gcc->ConnectorNumber].peer_svid_length == 0) {
        //*(char *)&msg->msgi = 0;
        goto out;
    }

    if (snc->EnterOrExit == 1) {
        dp_alt_enter_mode();
    } else {
        dp_alt_exit_mode();
    }

    //ucsi_errno = ERR_CMD_UNSUCCESS;
out:
    result = UCSI_COMMAND_SUCC;
    set_new_cam_ack();

    return result;
}
#endif

/*
 * @desc   : OPM get pdos from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
#if UCSI_PDOS_SUPPORT
static u8 ppm_get_pdos()
{
    //u8 DataLength;
    //u8 pdo_index;

    if (gp->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    result = UCSI_COMMAND_FAIL;
    port_index = gp->ConnectorNumber - 1;
    //DETECT_POWER_ON(port_index);

#ifdef ANXDEBUG
    ucsi_debug("Get %s : %s PDOOffset (%.bx), numOfPDO(%.bx), partner_pdo_length is %.bx\n",
        (gp->PartnerPDO == 0)?"connector":"partner", (gp->SourceOrSinkPDOs)?"source":"sink",
        gp->PDOOffset, gp->NumberOfPDOs,
        context[port_index].partner_pdo_length);
#endif
    if (gp->PartnerPDO == 0) {
        if (gp->SourceOrSinkPDOs == 1) {
            switch (gp->SourceCapabilities) {
            case SCTCURRENT_SUPPORTED_SOURCE_CAPABILITIES:
            case SCTMAXIMUM_SUPPORTED_SOURCE_CAPABILITIES:
            case SCTADVERTISED_CAPABILITIES:
                if (gp->PDOOffset <= 1) {
                    memcpy(&pd->PDO[0], &context[port_index].pd_src_pdo[0], 4);
                    DataLength = 4;
                } else {
                    pd->PDO[0] = 0;
                    pd->PDO[1] = 0;
                    pd->PDO[2] = 0;
                    pd->PDO[3] = 0;
                    DataLength = 0;
                }
                break;
            default:
                ucsi_errno = ERR_INVALID;
                set_error_data();
                break;
            }
        } else {
            if (gp->PDOOffset <= 1) {
                memcpy(&pd->PDO[0], &context[port_id].pd_snk_pdo, 4);
                //#ifdef EC_ANX_BIG_ENDIAN
                //SWAP_DWORD(&pd->PDO[pdo_index]);
                //#endif
                DataLength = 4;
            } else {
                pd->PDO[0] = 0;
                pd->PDO[1] = 0;
                pd->PDO[2] = 0;
                pd->PDO[3] = 0;
                DataLength = 0;
            }
        }
    } else {
        if (gp->PDOOffset * 4 < context[port_index].partner_pdo_length) {
            
            //memcpy((void *)&pd->PDO, &context[port_index].partner_pdo[4 * gp->PDOOffset],
            //    min((gp->NumberOfPDOs + 1)*4,(context[port_index].partner_pdo_length - (gp->PDOOffset * 4))));
            //DataLength = min((gp->NumberOfPDOs + 1)*4,(context[port_index].partner_pdo_length - (gp->PDOOffset * 4)));
            memcpy((void *)&pd->PDO, &context[port_index].partner_pdo[0],
                min((gp->NumberOfPDOs+1)*4,context[port_index].partner_pdo_length));
            DataLength = min((gp->NumberOfPDOs+1)*4,context[port_index].partner_pdo_length);
            
            #ifdef ANXDEBUG
            ucsi_debug("+++++++++++++++  executed to here, DataLength(%.bx).\n", DataLength);
            #endif
        } else {
            pd->PDO[0] = 0;
            pd->PDO[1] = 0;
            pd->PDO[2] = 0;
            pd->PDO[3] = 0;
            DataLength = 0;
        }

    }

    result = UCSI_COMMAND_SUCC;
    #ifdef ANXDEBUG
    ucsi_debug("***********   get PDO from OS : ");
    for (i = 0; i < DataLength; i++) {
        ucsi_debug("%.bx ", ((u8 *)&pd->PDO[0])[i]);
    }
    ucsi_debug("\n");
    #endif
out:
    get_pdos_ack();

    return result;
}
#endif

/*
 * @desc   : OPM get cable property from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
#if UCSI_CABLE_SUPPORT
static u8 ppm_get_cable_property()
{
    if (gcp->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    port_index = gcp->ConnectorNumber - 1;
    //ret = pop->ppm_cable_property(&pop->param, gcp->ConnectorNumber, (void *)&msg->msgi);
    S_CLEAR(cpd, struct cable_property_data);

    DETECT_POWER_ON(port_index);
    
    if ((context[port_index].pd_capability & PD_HAS_CABLE_VDO) &&
        (*(u32 *)&context[port_index].cv != 0)) {
        cpd->VBUSInCable = context[port_index].cv.vbus_thru_cable;
            switch (context[port_index].cv.vbus_current) {
            case VBUS_3A:
                cpd->bCurrentCapability = 60;
                break;
            case VBUS_5A:
                cpd->bCurrentCapability = 100;
                break;
            default:
                cpd->bCurrentCapability = 0;
                break;
            }
    
            cpd->CableType = 0; //Passive cable
            cpd->ModeSupport = 0;
            cpd->PlugEndType = context[port_index].cv.cable_type;
            if (context[port_index].cv.SSTX1_sup || context[port_index].cv.SSTX2_sup ||
                context[port_index].cv.SSRX1_sup || context[port_index].cv.SSRX2_sup)
                cpd->Directionality = 1;
            else
                cpd->Directionality = 0;
    
            //need more investigation
            cpd->bmSpeedSupported = 0;
            switch (context[port_index].cv.supper_speed_signal) {
            case SUPPER_SPEED_USB_2_0:
                cpd->bmSpeedSupported = (480 << 2) | 2; //480Mbps
                break;
            case SUPPER_SPEED_USB_3_1:
                cpd->bmSpeedSupported = (5 << 2) | 3; //5Gbps
                break;
            case SUPPER_SPEED_USB_3_1_2:
                cpd->bmSpeedSupported = (10 << 2) | 3; //10Gbps
                break;
            default:
                cpd->bmSpeedSupported = (12 << 2) | 2; //480Mbps
                break;
            }
            cpd->Latency = context[port_index].cv.latency;
    
            result = UCSI_COMMAND_SUCC;
            goto out;
        }
    
        //Need firmware support
        switch (context[port_index].ct) {
        case CABLE_TYPE_A:
            cpd->PlugEndType = PLUG_TYPEA;
            break;
        case CABLE_TYPE_B:
            cpd->PlugEndType = PLUG_TYPEB;
            break;
        case CABLE_TYPE_C:
            cpd->PlugEndType = PLUG_TYPEC;
            break;
        default:
            cpd->PlugEndType = PLUG_OTHER;
        }
    
        //Set default cable property
        cpd->bmSpeedSupported = (480 << 2) | 2;
        cpd->bCurrentCapability = 60; //3A
        cpd->VBUSInCable = 1;
        cpd->CableType = 0; //Passive cable
        cpd->Directionality = 0;
        cpd->ModeSupport = 0;
        cpd->Latency = 0x5; //40ns to 50ns ~5m
    
        result = UCSI_COMMAND_SUCC;
out:
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_WORD(&cpd->bmSpeedSupported);
    #endif

    get_cable_property_ack();

    return result;
}
#endif

/*
 * @desc   : OPM get specific error code from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_get_error_status()
{
    if (*(u16 *)&(esd.ErrorInformation) != 0) {
        result = UCSI_COMMAND_SUCC;
        get_error_status_ack();
        *(struct error_status_data *)&msg->msgi = esd;
        #ifdef EC_ANX_BIG_ENDIAN
        SWAP_WORD(&msg->msgi);
        #endif
    } else {
        ucsi_errno = ERR_INVALID;
        result = UCSI_COMMAND_FAIL;
        get_error_status_ack();
    }

    return result;

}

/*
 * @desc   : OPM get get connector status from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_get_connector_status()
{
    #ifdef ANXDEBUG
    ucsi_debug("********   connector number is %.bx **** \n", gcs->ConnectorNumber);
    #endif
    if (gcs->ConnectorNumber > PD_MAX_INSTANCE) {
        //printf("didn't find connector .\n");
        #ifdef HLK_RS2
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        result = UCSI_COMMAND_SUCC;
        get_connector_status_ack();
        context[0].csc.csc.Error = 1;
        ucsi_async_checking_timer = 250;
        return result;
        #else
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        result = UCSI_COMMAND_FAIL;
        get_connector_status_ack();
        return result;
        #endif
    }

    port_index = gcs->ConnectorNumber - 1;

    csd->ConnectStatus = context[port_index].anx_power_status & context[port_index].enable_report_power_on;
    if (csd->ConnectStatus) {
        csd->RequestDataObject = context[port_index].ucsi_partner_rdo;
        #ifdef EC_ANX_BIG_ENDIAN
        SWAP_DWORD(&csd->RequestDataObject);
        #endif

        if (ne.ConnectorPartnerChange) {
            if (context[port_index].ConnectorPartner)
                csd->ConnectorPartner = context[port_index].ConnectorPartner;
            else
                csd->ConnectorPartner = 1;

            switch (context[port_index].ucsi_connector_partner_type) {
            case CPT_AUDIO_ACC:
            case CPT_CAB_DEBUG_ACC:
                break;
            default:
                role = get_data_role();
                if (role == 0) {//UFP(DFP Attached)
                    context[port_id].ucsi_connector_partner_type = CPT_DFP_ATTACHED;
                } else {//DFP(UFP Attached)
                    if (context[port_id].ucsi_connector_partner_type == CPT_DFP_ATTACHED)
                        context[port_id].ucsi_connector_partner_type = CPT_UFP_ATTACHED;
                }
                break;
            }
            csd->ConnectorPartnerType = context[port_index].ucsi_connector_partner_type;
            #ifdef ANXDEBUG
            ucsi_debug("-------  connectorPartner flag %.bx, type %.bx, drole %.bx.\n",
                csd->ConnectorPartner, csd->ConnectorPartnerType, role);
            #endif
        }

        if (ne.PowerDirectionChange) {
        #ifdef ANXDEBUG
        ucsi_debug("======   port index(%.bx), anx_vbus_status is %.bx.\n",
            port_index, context[port_index].anx_vbus_status);
        #endif
        if (context[port_index].anx_power_role == 1)
            csd->PowerDirection = PD_PROVIDER;
        else
            csd->PowerDirection = PD_CONSUMER;
        }
        if (ne.BatteryChargingStatusChange) {
            csd->BatteryChargingStatus = context[port_index].BatteryCharging;
        }

        if (context[port_index].ucsi_partner_rdo != 0)
            csd->PowerOperationMode = PO_PD;
        else
            csd->PowerOperationMode = context[port_id].power_op_mode;
    }

    if (ne.ConnectChange && context[port_index].csc.csc.Connect) {
        csd->csc.Connect = 1;
        context[port_index].csc.csc.Connect = 0;
        //csd->ConnectStatus = context[port_index].anx_power_status;
        goto out;
    }

    if (csd->ConnectStatus == 0)
        goto out;

    if (ne.SupportedProviderCapabilitiesChange && context[port_index].csc.csc.SupportedProviderCap) {
        context[port_index].csc.csc.SupportedProviderCap = 0;
        csd->csc.SupportedProviderCap = 1;
        goto out;
    }

    if (ne.ExternalSupplyChange && context[port_index].csc.csc.ExternalSupply) {
            context[port_index].csc.csc.ExternalSupply = 0;
            csd->csc.ExternalSupply = 1;
        goto out;
    }

    if (ne.PowerDirectionChange && context[port_index].csc.csc.PowerDirection) {
        context[port_index].csc.csc.PowerDirection = 0;
        csd->csc.PowerDirection = 1;
        goto out;
    }

    if (ne.ConnectorPartnerChange && context[port_index].csc.csc.ConnectorPartner) {
        context[port_index].csc.csc.ConnectorPartner = 0;
        csd->csc.ConnectorPartner = 1;
        goto out;
    }

	if (ne.PowerOperationModeChange && context[port_index].csc.csc.PowerOperationMode) {
        context[port_index].csc.csc.PowerOperationMode = 0;
        csd->csc.PowerOperationMode = 1;
        goto out;
	}

    if (ne.PDResetComplete && context[port_index].csc.csc.PDResetComplete) {
        context[port_index].csc.csc.PDResetComplete = 0;
        csd->csc.PDResetComplete = 1;
        goto out;
    }

    if (ne.SupportedProviderCapabilitiesChange && context[port_index].csc.csc.SupportedProviderCap) {
        context[port_index].csc.csc.SupportedProviderCap = 0;
        csd->csc.SupportedProviderCap = 1;
        goto out;
    }

    if (ne.Error && context[port_index].csc.csc.Error) {
        context[port_index].csc.csc.Error = 0;
        csd->csc.Error = 1;
        goto out;
    }

    //Fix JIRA LBT-429
    if (ne.SupportedCAMChange && context[port_index].csc.csc.SupportedCAM) {
        context[port_index].csc.csc.SupportedCAM = 0;
        csd->csc.SupportedCAM = 1;
        goto out;
    }

    if (ne.BatteryChargingStatusChange && context[port_index].csc.csc.BatteryChargingStatus) {
        context[port_index].csc.csc.BatteryChargingStatus = 0;
        csd->csc.BatteryChargingStatus = 1;
        goto out;
    }

    if (ne.NegotiatedPowerLevelChange && context[port_index].csc.csc.NegotiatedPowerLevel) {
        context[port_index].csc.csc.NegotiatedPowerLevel = 0;
        csd->csc.NegotiatedPowerLevel = 1;
    }

    if (context[port_index].csc.connector_change_indicator) {
        #ifdef ANXDEBUG
        ucsi_debug("????   port index(%.bx), still have unsupported event pending %x, just ignore.\n",
            port_index, context[port_index].csc.connector_change_indicator);
        #endif
        context[port_index].csc.connector_change_indicator = 0;
    }

out:
    result = UCSI_COMMAND_SUCC;
    get_connector_status_ack();
	if (context[port_index].csc.connector_change_indicator)
        //Fix JIRA LBT-429
		ucsi_async_checking_timer = UCSI_CONNECTOR_REPORT_TIME;

    /* Fix JIRA LBT-429, waiting for system send GET_PDO command */
    if (csd->csc.Connect == 1)
        ucsi_async_checking_timer = 1000;

    return result;
}

#if UCSI_SET_PDM_SUPPORT
/*
 * @desc   : OPM set power direction mode for PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_set_pdm()
{
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_WORD((XBYTE *)spdm + 2);
    #endif
    if (spdm->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    port_index = spdm->ConnectorNumber - 1;
    if (context[port_index].dfp_only && spdm->PowerDirectionMode != PW_PROVIDER) {
        ucsi_errno = ERR_INVALID;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }
    context[port_index].power_operation_mode_pdm = 0;

    result = UCSI_COMMAND_SUCC;
    //context[port_index].power_operation_mode_pdm = (((((XBYTE *)spdm)[3])&0x80) >> 6) | (((((XBYTE *)spdm)[2])&0x3) << 1);;
    if ((((XBYTE *)spdm)[3])&0x80)
        context[port_index].power_operation_mode_pdm |= PW_PROVIDER;
    if ((((XBYTE *)spdm)[2])&0x1)
        context[port_index].power_operation_mode_pdm |= PW_CONSUMER;
    if ((((XBYTE *)spdm)[2])&0x2)
        context[port_index].power_operation_mode_pdm |= PW_DRP;

    switch (context[port_index].power_operation_mode_pdm) {
    case 1: //Provider
        context[port_index].trysrc_enable = 1;
        break;
    case 2: //Consumer
        context[port_index].trysnk_enable = 1;
        break;
    case 4: //Provider/Consumer
        context[port_index].trysrc_enable = 0;
        context[port_index].trysnk_enable = 0;
        break;
    case 5: //DUAL + Provider
        context[port_index].trysrc_enable = 1;
        break;
    case 6: //DUAL + Consumer
        context[port_index].trysnk_enable = 1;
        break;
    default:
        ucsi_errno = ERR_INVALID;
        set_error_data();
        result = UCSI_COMMAND_FAIL;
        break;
    }

    set_pdr_ack();

    return result;
}
#endif

static u8 ppm_set_pdr_process()
{
    u8 ret = UCSI_COMMAND_FAIL;

    if (ucsi_command_process_delay == UCSI_PDR_WAITING_TIME)
        send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
    if (ucsi_command_process_delay) {
        if (--ucsi_command_process_delay == 0) {
            port_id = port_index;
            switch(get_power_role()) {
            case 1: //DFP
                role = PW_PROVIDER;
                break;
            case 0: //UFP
                role = PW_CONSUMER;
                break;
            }
            if (context[port_index].power_operation_mode_pdr & role) {
                set_uor_ack();
                cancel_all_timer();
                context[port_id].csc.connector_change_indicator = 0;
                ret = UCSI_COMMAND_SUCC;
            } else {
                ucsi_errno = ERR_NEGOTIATION_FAIL;
                set_error_data();
                ret = UCSI_COMMAND_FAIL;
            }
        } else {
            ret = UCSI_COMMAND_CONTINUE;
        }
    }
    return ret;
}
/*
 * @desc   : OPM set power direction role for PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_set_pdr()
{
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_WORD((XBYTE *)spdr + 2);
    #endif

    if (spdr->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    port_index = spdr->ConnectorNumber - 1;
    DETECT_POWER_ON(port_index);
    context[port_index].power_operation_mode_pdr = 0;
	//context[port_index].power_operation_mode_pdr = (((((XBYTE *)spdr)[3])&0x80) >> 6) | (((((XBYTE *)spdr)[2])&0x3) << 1);;
    if ((((XBYTE *)spdr)[3])&0x80)
        context[port_index].power_operation_mode_pdr |= PW_PROVIDER;
    if ((((XBYTE *)spdr)[2])&0x1)
        context[port_index].power_operation_mode_pdr |= PW_CONSUMER;
    if ((((XBYTE *)spdr)[2])&0x2)
        context[port_index].power_operation_mode_pdr |= PW_DRP;

    if (context[port_index].dfp_only && context[port_index].power_operation_mode_pdr != PW_PROVIDER) {
        ucsi_errno = ERR_INVALID;
        set_error_data();
        result = UCSI_COMMAND_FAIL;
        goto out;
    }

    port_id = port_index;

    send_source_sink_capability();

    switch(get_power_role()) {
    case 1: //DFP
        role = PW_PROVIDER;
        break;
    case 0: //UFP
        role = PW_CONSUMER;
        break;
    }

    #ifdef ANXDEBUG
    ucsi_debug("___________ pdr  %.bx, %.bx, %.bx, role %.bx.\n", ((XBYTE *)spdr)[2], ((XBYTE *)spdr)[3],
        context[port_index].power_operation_mode_pdr, role);
    #endif

    result = UCSI_COMMAND_SUCC;
    switch (context[port_index].power_operation_mode_pdr) {
    case PW_PROVIDER:
        if (role == PW_CONSUMER) {
            //send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(POWER_ROLE_SWAP_DELAY_MS);
                port_id = port_index;
                if (get_power_role() != 1) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                //Fix JIRA LBT-412
                ucsi_command_process_delay = UCSI_PDR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        }
        break;
    case PW_CONSUMER:
        if (role == PW_PROVIDER) {
            //send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(POWER_ROLE_SWAP_DELAY_MS);
                if (get_power_role() != 0) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                //Fix JIRA LBT-412
                ucsi_command_process_delay = UCSI_PDR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        }
        break;
    case PW_DRP:
        //Do nothing
        result = UCSI_COMMAND_SUCC;
        break;
    case PW_DRP_CONSUMER:
        if (role == PW_PROVIDER) {
            //send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(POWER_ROLE_SWAP_DELAY_MS);
                if (get_power_role() != 0) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                //Fix JIRA LBT-412
                ucsi_command_process_delay = UCSI_PDR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        }
        break;
    case PW_DRP_PROVIDER:
        if (role == PW_CONSUMER) {
            //send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_PSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(POWER_ROLE_SWAP_DELAY_MS);
                port_id = port_index;
                if (get_power_role() != 1) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                ucsi_command_process_delay = UCSI_PDR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        }
        break;
    default:
        ucsi_errno = ERR_UNRECOGNIZED;
		result = UCSI_COMMAND_FAIL;
        break;
    }

out:
    set_pdr_ack();

    return result;
}

#if UCSI_SET_UOM_SUPPORT
/*
 * @desc   : OPM set usb operation mode for PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_set_uom()
{
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_WORD((XBYTE *)suom + 2);
    #endif

    if (suom->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    port_index = suom->ConnectorNumber - 1;

    if (context[port_index].dfp_only && suom->USBOperationMode != OP_DFP) {
        ucsi_errno = ERR_INVALID;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }
    context[port_index].data_operation_mode_uom = 0;

    result = UCSI_COMMAND_SUCC;
    switch (suom->USBOperationMode) {
    case 1: //UFP
    case 2: //DRP
    case 4: //DRP
    case 5: //DRP + DFP
    case 6: //DRP + UFP
        context[port_index].data_operation_mode_uom = suom->USBOperationMode;
        #ifdef ANXDEBUG
        ucsi_debug("___________ 1  data operation mode is %.bx, %.bx.\n", suom->USBOperationMode,
            context[port_index].data_operation_mode_uom);
        if ((((XBYTE *)suom)[3])&0x80)
            context[port_index].data_operation_mode_uom |= OP_DFP;
        if ((((XBYTE *)suom)[2])&0x1)
            context[port_index].data_operation_mode_uom |= OP_UFP;
        if ((((XBYTE *)suom)[2])&0x2)
            context[port_index].data_operation_mode_uom |= OP_DRP;
        //context[port_index].data_operation_mode_uom = (((((XBYTE *)suom)[3])&0x80) >> 6) | (((((XBYTE *)suom)[2])&0x3) << 1);
        ucsi_debug("___________ 2  %.bx, %.bx, %.bx.\n", ((XBYTE *)suom)[2], ((XBYTE *)suom)[3],
            context[port_index].data_operation_mode_uom);
        #endif
        break;
    default:
        ucsi_errno = ERR_INVALID;
        set_error_data();
        result = UCSI_COMMAND_FAIL;
        break;
    }

    RamDebug(UCSI_COMMAND_UOM);
    RamDebug(context[port_index].data_operation_mode_uom);
    set_uor_ack();
    if ((result == UCSI_COMMAND_SUCC) && (context[port_index].anx_power_status == 0))
    {
    	RamDebug(0xE5);
        chip_power_on(port_index);
    }

    return result;
}
#endif

static u8 ppm_set_uor_process()
{
    u8 ret = UCSI_COMMAND_FAIL;

    if (ucsi_command_process_delay == UCSI_UOR_WAITING_TIME)
        send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
    if (ucsi_command_process_delay) {
        if (--ucsi_command_process_delay == 0) {
            port_id = port_index;
            switch (get_data_role()) {
            case 1: //DFP
                role = OP_DFP;
                context[port_id].ucsi_connector_partner_type = CPT_UFP_ATTACHED;
                break;
            case 0: //UFP
                role = OP_UFP;
                context[port_id].ucsi_connector_partner_type = CPT_DFP_ATTACHED;
                break;
            }
            if (context[port_id].data_operation_mode_uor & role) {
                if (context[port_id].csc.csc.PowerDirection == 1) {
                    context[port_id].csc.connector_change_indicator = 0;
                    context[port_id].csc.csc.PowerDirection = 1;
                } else {
                    context[port_id].csc.connector_change_indicator = 0;
                    cancel_all_timer();
                }
                set_uor_ack();
                ret = UCSI_COMMAND_SUCC;
            } else {
                ucsi_errno = ERR_NEGOTIATION_FAIL;
                set_error_data();
                ret = UCSI_COMMAND_FAIL;
            }
        } else {
            ret = UCSI_COMMAND_CONTINUE;
        }
    }
    return ret;
}
/*
 * @desc   : OPM set usb operation role for PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_set_uor()
{
    //u8 data_role = 0;
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_WORD((XBYTE *)suor + 2);
    #endif

    if (suor->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    port_index = suor->ConnectorNumber - 1;
    
    DETECT_POWER_ON(port_index);
    context[port_index].data_operation_mode_uor = 0;

    //context[port_index].data_operation_mode_uor = (((((XBYTE *)suor)[3])&0x80) >> 6) | (((((XBYTE *)suor)[2])&0x3) << 1);
    if ((((XBYTE *)suor)[3])&0x80)
        context[port_index].data_operation_mode_uor |= OP_DFP;
    if ((((XBYTE *)suor)[2])&0x1)
        context[port_index].data_operation_mode_uor |= OP_UFP;
    if ((((XBYTE *)suor)[2])&0x2)
        context[port_index].data_operation_mode_uor |= OP_DRP;

    if (context[port_index].dfp_only && context[port_index].data_operation_mode_uor != OP_DFP) {
        ucsi_errno = ERR_INVALID;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }
    port_id = port_index;
    switch (get_data_role()) {
    case 1: //DFP
        role = OP_DFP;
        break;
    case 0: //UFP
        role = OP_UFP;
        break;
    }

    result = UCSI_COMMAND_SUCC;
    port_id = port_index;
    #ifdef ANXDEBUG
    ucsi_debug("___________ 3  %.bx, %.bx, %.bx, role %.bx.\n", ((XBYTE *)suor)[2], ((XBYTE *)suor)[3],
        context[port_index].data_operation_mode_uor, role);
    #endif

    send_source_sink_capability();

    switch (context[port_index].data_operation_mode_uor) {
    case OP_DFP:
        if (role == OP_UFP) {
            //send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(DATA_ROLE_SWAP_DELAY_MS);

                if (get_data_role() != OP_DFP) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                ucsi_command_process_delay = UCSI_UOR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        } else
            result = UCSI_COMMAND_SUCC;
        break;
    case OP_UFP:
        if (role == OP_DFP) {
            //send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(DATA_ROLE_SWAP_DELAY_MS);

                if (get_data_role() != OP_UFP) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                ucsi_command_process_delay = UCSI_UOR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        } else
            result = UCSI_COMMAND_SUCC;
        break;
    case OP_DRP:
        //Do nothing
        result = UCSI_COMMAND_SUCC;
        break;
    case OP_DRP_DFP:
        if (role == OP_UFP) {
            //send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(DATA_ROLE_SWAP_DELAY_MS);
                if (get_data_role() != OP_DFP) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                ucsi_command_process_delay = UCSI_UOR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        } else
        result = UCSI_COMMAND_SUCC;
        break;
    case OP_DRP_UFP:
        if (role == OP_DFP) {
            //send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
            if (!(context[port_index].pd_capability & PD_HAS_RETURN_STATUS)) {
                send_pd_msg(TYPE_DSWAP_REQ, 0, 0, SOP_TYPE);
                DelayXms(DATA_ROLE_SWAP_DELAY_MS);
                if (get_data_role() != OP_UFP) {
                    ucsi_errno = ERR_NEGOTIATION_FAIL;
                    set_error_data();
                    result = UCSI_COMMAND_FAIL;
                }
            } else {
                ucsi_command_process_delay = UCSI_UOR_WAITING_TIME;
                result = UCSI_COMMAND_CONTINUE;
            }
        } else
        result = UCSI_COMMAND_SUCC;
        break;
    }

    set_uor_ack();

    return result;
}

/*
 * @desc   : OPM get connector capability from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_get_connector_capability()
{
    if (gcc->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    port_index = gcc->ConnectorNumber - 1;

    S_CLEAR(ccd, struct connector_capability_data);

    //DETECT_POWER_ON(port_index);

    if (context[port_index].dfp_only)
        ccd->OperationMode = OM_DFP_ONLY | OM_USB2_SUP | OM_USB3_SUP |
                OM_AUDIO_SUP | OM_DEBUG_ACC_SUP;
    else
        #if 0
        ccd->OperationMode = OM_DRP | OM_DFP_ONLY | OM_UFP_ONLY |
                OM_DEBUG_ACC_SUP | OM_USB2_SUP | OM_AUDIO_SUP |
                OM_USB3_SUP;
        #else
        ccd->OperationMode = OM_DRP | OM_DFP_ONLY | OM_UFP_ONLY |
            OM_USB2_SUP | OM_USB3_SUP;
        #endif

    #if UCSI_ALT_SUPPORT
    ccd->OperationMode |= OM_ALT_SUP;
    #endif
    ccd->Provider = 1;
    if (!context[port_index].dfp_only)
        ccd->Consumer = 1;


    result = UCSI_COMMAND_SUCC;
    get_connector_capability_ack();

    return result;
}

/*
 * @desc   : OPM get capability from PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_get_capability()
{
    S_CLEAR(cci, struct cci_status);
    S_CLEAR(cd, struct capability_data);

    cd->bmAttributes.v1.DisabledStateSupport = 0;
    cd->bmAttributes.v1.BatteryChaging = 1;
    cd->bmAttributes.v1.USBPowerDelivery = 1;
    cd->bmAttributes.v1.USBTypeCCurrent = 1;
    cd->bmAttributes.v1.bmPowerSource.v1.UsesVBUS = 1;
    cd->bNumConnectors = PD_MAX_INSTANCE;

    cd->bmOptionalFeatures1 = CMD_OPTIONAL_SUPPORT;
#if UCSI_ALT_SUPPORT
    //support 2 alt mode, usb 3.x, DP.
    cd->bNumAltModes = 2;
#endif
    cd->bcdBCVersion = 0;
    cd->bcdPDVersion = PD_VERSION;
    cd->bcdUSBTypeCVersion = TYPEC_VERSION;
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_WORD(&cd->bcdPDVersion);
    SWAP_WORD(&cd->bcdUSBTypeCVersion);
    #endif

    result = UCSI_COMMAND_SUCC;
    get_capability_ack();

    return UCSI_COMMAND_SUCC;
}

static void ppm_init()
{
    for (i = 0; i < PD_MAX_INSTANCE; i++)
	{
		RamDebug(0xE4);
        chip_power_on(i);
		RamDebug(0xE8);
        S_CLEAR(&context[i].csc.csc, struct connector_status_change);
        context[i].power_operation_mode_pdm = PW_DRP;
        context[i].power_operation_mode_pdr = PW_DRP;
		context[i].data_operation_mode_uom = OP_DRP;
        context[i].data_operation_mode_uor = OP_DRP;
		RamDebug(0xE9);
    }
}

static void ppm_init_source_sink_capability()
{
    for (port_id = 0; port_id < PD_MAX_INSTANCE; port_id++)
        if (context[port_id].anx_power_status == 1)
            send_source_sink_capability();
}

/*
 * @desc   : OPM reset PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_reset()
{
    S_CLEAR(&ne, struct notification_enable_status);

    cancel_all_timer();
    ppm_init();
    ppm_init_source_sink_capability();
    result = UCSI_COMMAND_SUCC;
    reset_ack();
    memcpy((XBYTE *)&xUCSI_DS_CCI0, (XBYTE *)cci, 4);   
    //PPM_RESET just need to to IDLE status
    //GOTO_STATE(PPM_IDLE_ND);
    return UCSI_COMMAND_SUCC;
}

/*
 * @desc   : OPM CANCEL command
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_cancel()
{
    if (current_pending_command) {
        result = UCSI_COMMAND_SUCC;
        cancel_ack();
    } else {
        result = UCSI_COMMAND_FAIL;
        cancel_ack();
        result = UCSI_COMMAND_NOT_NOTIFY;
    }

    return result;
}

/*
 * @desc   : OPM ACK to PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_ack_cc_ci()
{
#ifdef ANXDEBUG
    ucsi_debug("%s:%d ConnectorChangeAck(%.bx), CommandCompletedAck(%.bx).\n",
            __func__, __LINE__,
            acc->ConnectorChangeAcknowledge,
            acc->CommandCompletedAcknowledge);
#endif
    result = UCSI_COMMAND_SUCC;
    ack_cc_ci_ack();

    if (acc->ConnectorChangeAcknowledge)
        connector_change_need_ack = 0;

    if (acc->CommandCompletedAcknowledge)
        command_complete_need_ack = 0;

    return UCSI_COMMAND_SUCC;
}
static u8 ppm_connector_reset_process()
{
    u8 ret = UCSI_COMMAND_FAIL;
    if (ucsi_command_process_delay) {
        if (--ucsi_command_process_delay == 0) {
            if (context[port_index].csc.csc.PDResetComplete) {
                connector_reset_ack();
                ret = UCSI_COMMAND_SUCC;
            } else {
                ucsi_errno = ERR_UNRECOGNIZED;
                set_error_data();
                ret = UCSI_COMMAND_FAIL;
            }
        } else {
            ret = UCSI_COMMAND_CONTINUE;
        }
    }
    return ret;
}

/*
 * @desc   : OPM reset connector on PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_connector_reset()
{
    port_index = cr->ConnectorNumber - 1;

    if (cr->ConnectorNumber > PD_MAX_INSTANCE) {
        ucsi_errno = ERR_NON_EXIST;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }
    //DETECT_POWER_ON(port_index);

    if (IS_BATTERY1_OUT() || (IS_BATTERY1_IN() && BAT1_RSOC <= 3)) {
        ucsi_errno = ERR_CMD_UNSUCCESS_DUE_TO_DEAD_BATTERY;
        set_error_data();
        return UCSI_COMMAND_FAIL;
    }

    result = UCSI_COMMAND_SUCC;
        port_id = port_index;

    if (cr->HardReset)
        send_pd_msg(TYPE_HARD_RST, NULL, 0, SOP_TYPE);
    else
        send_pd_msg(TYPE_SOFT_RST, NULL, 0, SOP_TYPE);

    connector_reset_ack();

    return result;
}

/*
 * @desc   : OPM set notification enable on PPM
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_set_notification_enable()
{
    ne.CommandCompleted = sne->status.CommandCompleted;
    ne.ExternalSupplyChange = sne->status.ExternalSupplyChange;
    ne.PowerOperationModeChange = sne->status.PowerOperationModeChange ;
    ne.SupportedProviderCapabilitiesChange = sne->status.SupportedProviderCapabilitiesChange ;
    ne.NegotiatedPowerLevelChange = sne->status.NegotiatedPowerLevelChange ;
    ne.PDResetComplete = sne->status.PDResetComplete ;
    ne.SupportedCAMChange = sne->status.SupportedCAMChange ;
    ne.BatteryChargingStatusChange = sne->status.BatteryChargingStatusChange ;
    ne.ConnectorPartnerChange = sne->status.ConnectorPartnerChange ;
    ne.PowerDirectionChange = sne->status.PowerDirectionChange ;
    ne.ConnectChange = sne->status.ConnectChange ;
    ne.Error = sne->status.Error ;

    result = UCSI_COMMAND_SUCC;
    set_notification_enable_ack();

    return UCSI_COMMAND_SUCC;
}

static void ppm_alert(void)
{
    #ifdef ANXDEBUG
    ucsi_debug("%s:%d, ------------- PPM alert OPM, cci -> %.bx %.bx %.bx %.bx.\n", __func__, __LINE__,
        xUCSI_DS_CCI0, xUCSI_DS_CCI1, xUCSI_DS_CCI2, xUCSI_DS_CCI3);
    #endif
    RamDebug(0xAA);
    RamDebug(0x77);
    ucsi_ppm_notify_opm();
}
static void ucsi_command_complete_notify_opm(void)
{
    if (ne.CommandCompleted) {
       memcpy((XBYTE *)&xUCSI_DS_CCI0, (XBYTE *)cci, 4);
       memcpy((XBYTE *)&xUCSI_DS_MGI, (XBYTE *)msg->msgi, 16);
	   #ifdef ANXDEBUG
       ucsi_debug("++++    CTRL message -> ");
	   for (i = 0; i < 16; i++) {
	       ucsi_debug("%.bx ", xUCSI_DS_MGI[i]);
       }
	   ucsi_debug("\n");
       #endif
       ppm_alert();
   }
}
static u8 ucsi_command_process_timeout_checking(void)
{
    if (current_pending_command_timeout) {
        if (--current_pending_command_timeout == 0) {
        #ifdef ANXDEBUG
            ucsi_debug("%s:%d command timeout, pending command(%.bx), state(%.bx).\n",
                __func__, __LINE__, current_pending_command, ucsi_ppm_state);
        #endif
            ucsi_errno = ERR_UNRECOGNIZED;
            set_error_data();
            ucsi_command_complete_notify_opm();
            return UCSI_COMMAND_SUCC;
        }
    }
    return UCSI_COMMAND_FAIL;
}
void cancel_all_timer()
{
    ucsi_async_checking_timer = 0;
    ucsi_async_restore = 0;
}
static void async_notify_opm(void)
{
    S_CLEAR(cci, struct cci_status);
    if ((ucsi_async_restore >= 5) && (ucsi_async_checking_timer == 0)) {
        ucsi_async_checking_timer = ucsi_async_restore;
        ucsi_async_restore = 0;
        #ifdef ANXDEBUG
        ucsi_debug("%s:%d ucsi reset async timer as %.bx.\n",
            __func__, __LINE__, ucsi_async_checking_timer);
        #endif
    }
    if (ucsi_async_checking_timer) {
        if (--ucsi_async_checking_timer == 0) {
            ucsi_ppm_connector_change_detect();
            #ifdef ANXDEBUG
            ucsi_debug("%s:%d ucsi connector change (%.bx) async notify opm.\n",
                __func__, __LINE__, cci->ConnectorChangeIndicator);
            #endif
            if (cci->ConnectorChangeIndicator) {
                ucsi_connector_change_notify_opm(0, cci->ConnectorChangeIndicator, 0);
                #ifdef ANXDEBUG
                #endif
            }
        }
    }
}
/*
 * @desc   : PPM IDLE processing
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_nd_process(void)
{
    u8 ret = UCSI_COMMAND_FAIL;
    switch (current_pending_command) {
    case UCSI_COMMAND_NOTIFICATION_ENABLE:
        ppm_set_notification_enable();

        if (ne.CommandCompleted) {
            wait_for_ack_back = 250;
            GOTO_STATE(PPM_WCCA);
            ret = UCSI_COMMAND_SUCC;
        }
        break;
    case UCSI_COMMAND_PPM_RESET:
        ppm_reset();
        GOTO_STATE(PPM_IDLE_ND);
        break;
    case UCSI_COMMAND_RESERVED:
        return ret;
        break;
    default:
        break;
    }

    current_pending_command = 0;
#ifdef ANXDEBUG
    ucsi_debug("%s:%d UCSI state(ND), Command(%d).\n",
            __func__, __LINE__, sne->header.Command);
#endif
    if (ret == UCSI_COMMAND_SUCC)
        ucsi_command_complete_notify_opm();
    return ret;
}

/*
 * @desc   : PPM process state machine processing
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static void ppm_pc_process()
{
    u8 ret = UCSI_COMMAND_FAIL;

    ret = ucsi_command_process_timeout_checking();
    if (ret == UCSI_COMMAND_SUCC) {
        current_pending_command = 0;
        wait_for_ack_back = 250;
        GOTO_STATE(PPM_WCCA);
        return;
    }
    #ifdef ANXDEBUG
        ucsi_debug("%s:%d ucsi pc process.\n",
            __func__, __LINE__);
    #endif
    switch (ch->Command) {
    default:
#ifdef ANXDEBUG
        ucsi_debug("%s:%d Warning: received unsupported command(0x%x)"
                " at PPM_PC state.\n",
                __func__, __LINE__, ch->Command);
#endif
        ucsi_errno = ERR_UNRECOGNIZED;
        set_error_data();
        ret = UCSI_COMMAND_FAIL;
        break;
    case UCSI_COMMAND_CONNECTOR_RESET:
        ret = ppm_connector_reset_process();
        break;
    case UCSI_COMMAND_UOR:
        ret = ppm_set_uor_process();
        break;
    case UCSI_COMMAND_PDR:
        ret = ppm_set_pdr_process();
        break;
    }

    if (ret == UCSI_COMMAND_CONTINUE)
        return;
    current_pending_command = 0;
    ucsi_command_complete_notify_opm();
    wait_for_ack_back = 250;
    GOTO_STATE(PPM_WCCA);
}

/*
 * @desc   : PPM busy state machine processing
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_busy_process(u8 Command)
{
    u8 ret = UCSI_COMMAND_FAIL;

    S_CLEAR(cci, sizeof(struct cci_status));
    switch (Command) {
    default:
#ifdef ANXDEBUG
        ucsi_debug("%s:%d Warning: received unsupported command(0x%x)"
                " from OPM at PPM_BUSY state.\n",
                __func__, __LINE__, ch->Command);
#endif
        busy_ack();
        ret = UCSI_COMMAND_SUCC;
        break;
    case UCSI_COMMAND_PPM_RESET:
        ppm_reset();
        GOTO_STATE(PPM_IDLE_ND);
        ret = UCSI_COMMAND_NOT_NOTIFY;
        current_pending_command = 0;
        break;
    case UCSI_COMMAND_ACK_CC_CI:
        ret = ppm_ack_cc_ci();
        break;
    case UCSI_COMMAND_CANCEL:
        ret = ppm_cancel();
        GOTO_STATE(PPM_IDLE_NE);
        current_pending_command = 0;
        break;
    }
    if (ret != UCSI_COMMAND_NOT_NOTIFY)
        ucsi_command_complete_notify_opm();

    return ret;
}

/*
 * @desc   : ASYNC event wait for OPM ACK state machine processing
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_waea_process()
{
    u8 ret = UCSI_COMMAND_FAIL;

    switch (ch->Command) {
    case UCSI_COMMAND_ACK_CC_CI:
        ret = ppm_ack_cc_ci();
        GOTO_STATE(PPM_IDLE_NE);
        break;
    case UCSI_COMMAND_PPM_RESET:
        ppm_reset();
        ret = UCSI_COMMAND_NOT_NOTIFY;
        GOTO_STATE(PPM_IDLE_ND);
        break;
    case UCSI_COMMAND_GET_CONNECTOR_STATUS:
        ret = ppm_get_connector_status();
        GOTO_STATE(PPM_IDLE_NE);
        break;
    default:
#ifdef ANXDEBUG
        ucsi_debug("%s:%d, recevied not ACK_CC_CI command, continue waiting.\n",
                __func__, __LINE__);
#endif
        break;
    }

    current_pending_command = 0;
    if (UCSI_COMMAND_SUCC)
        ucsi_command_complete_notify_opm();
    return ret;
}


/*
 * @desc   : PPM wait for OPM ACK state machine processing
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_wcca_process()
{
    u8 ret = UCSI_COMMAND_FAIL;

    current_pending_command_timeout = 0;
    switch (ch->Command) {
    case UCSI_COMMAND_ACK_CC_CI:
        ret = ppm_ack_cc_ci();
        GOTO_STATE(PPM_IDLE_NE);
        break;
    case UCSI_COMMAND_PPM_RESET:
        ret = ppm_reset();
        GOTO_STATE(PPM_IDLE_ND);
        break;
    default:
        wait_for_ack_back--;
        if ((wait_for_ack_back == 120) || (wait_for_ack_back == 1)) {
            #ifdef ANXDEBUG
            ucsi_debug(" ack timeout, re-notify OPM.\n");
            #endif
            ucsi_command_complete_notify_opm();
        } else if (wait_for_ack_back == 0) {
            #ifdef ANXDEBUG
            ucsi_debug(" ack timeout, goto idle notification enable status.\n");
            #endif
            GOTO_STATE(PPM_IDLE_NE);
        }
        return ret;
#ifdef ANXDEBUG
        ucsi_debug("%s:%d, recevied not ACK_CC_CI command, continue waiting.\n",
                __func__, __LINE__);
#endif
        break;
    }

    current_pending_command = 0;
    if (UCSI_COMMAND_SUCC)
        ucsi_command_complete_notify_opm();
    return ret;
}

/*
 * @desc   : PPM main process
 *           PPM finish all required command processing here
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
static u8 ppm_ne_process()
{
    u8 ret = UCSI_COMMAND_FAIL;



    switch (ch->Command) {
    case UCSI_COMMAND_PPM_RESET:
        ppm_reset();
        GOTO_STATE(PPM_IDLE_ND);
        ret = UCSI_COMMAND_NOT_NOTIFY;
        break;
    case UCSI_COMMAND_CANCEL:
        //Just drop CANCEL command
        ret = UCSI_COMMAND_NOT_NOTIFY;
        //ret = cancel_ack(cci, UCSI_COMMAND_SUCC);
        break;
    case UCSI_COMMAND_CONNECTOR_RESET:
        ret = ppm_connector_reset();
        break;
    case UCSI_COMMAND_ACK_CC_CI:
        ret = ppm_ack_cc_ci();
        break;
    case UCSI_COMMAND_NOTIFICATION_ENABLE:
        ret = ppm_set_notification_enable();
        break;
    case UCSI_COMMAND_CAPABILITY:
        ret = ppm_get_capability();
        break;
    case UCSI_COMMAND_CONNECTOR_CAPABILITY:
        ret = ppm_get_connector_capability();
        break;
#if UCSI_SET_UOM_SUPPORT
    case UCSI_COMMAND_UOM:
        ret = ppm_set_uom();
        break;
#endif
    case UCSI_COMMAND_UOR:
        ret = ppm_set_uor();
        break;
#if UCSI_SET_PDM_SUPPORT
    case UCSI_COMMAND_PDM:
        ret = ppm_set_pdm();
        break;
#endif
    case UCSI_COMMAND_PDR:
        ret = ppm_set_pdr();
        break;
#if UCSI_ALT_SUPPORT
    case UCSI_COMMAND_GET_ALTERNATE_MODES:
        ret = ppm_get_alt_modes();
        break;
#endif
#if UCSI_ALT_SUPPORT
    case UCSI_COMMAND_GET_CAM_SUPPORTED:
        ret = ppm_get_cam_supported();
        break;
#endif
#if UCSI_ALT_SUPPORT
    case UCSI_COMMAND_GET_CURRENT_CAM:
        ret = ppm_get_current_cam();
        break;
#endif
#if UCSI_ALT_OR_SUPPORT
    case UCSI_COMMAND_SET_NEW_CAM:
        ret = ppm_set_new_cam();
        break;
#endif
#if UCSI_PDOS_SUPPORT
    case UCSI_COMMAND_GET_PDOS:
        ret = ppm_get_pdos();
        break;
#endif
#if UCSI_CABLE_SUPPORT
    case UCSI_COMMAND_GET_CABLE_PROPERTY:
        ret = ppm_get_cable_property();
        break;
#endif
    case UCSI_COMMAND_GET_CONNECTOR_STATUS:
        ret = ppm_get_connector_status();
        break;
    case UCSI_COMMAND_GET_ERROR_STATUS:
        ret = ppm_get_error_status();
        break;
    case UCSI_COMMAND_RESERVED:
        async_notify_opm();
        return ret;
        break;
    default:
        ret = unsupport_command_process();
#ifdef ANXDEBUG
        ucsi_debug("%s:%d Cannot support command(0x%x) from OPM.\n",
            __func__, __LINE__, ch->Command);
#endif
        break;
    }

    switch (ret) {
    case UCSI_COMMAND_NOT_NOTIFY:
        current_pending_command = 0;
        break;
    case UCSI_COMMAND_CONTINUE:
        #if UCSI_COMMAND_RETURN_BUSY_ACK
        //busy_ack();
        //ucsi_command_complete_notify_opm();
#endif
        GOTO_STATE(PPM_PC);
        break;
    //case UCSI_COMMAND_BUSY:
    case UCSI_COMMAND_SUCC:
    case UCSI_COMMAND_FAIL:
        wait_for_ack_back = 250;
        GOTO_STATE(PPM_WCCA);
        current_pending_command = 0;
        ucsi_command_complete_notify_opm();
        break;
    }


    return ret;
}

/*
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
u8 ucsi_connector_change_notify_opm(u8 ack, u8 connector_index, u8 wait_for_ack)
{
    u8 invalid = 1;

    //Ignore notify OPM under NOTIFICATION DISABLE STAE
    if (ucsi_ppm_state == PPM_IDLE_ND)
        return UCSI_COMMAND_FAIL;

    //If current PPM is not under PPM_IDLE_NE, no need notify OPM.
    if ((ack == 0) && (wait_for_ack == 0) &&
        (ucsi_ppm_state != PPM_IDLE_NE))
        return UCSI_COMMAND_SUCC;

    S_CLEAR(cci, struct cci_status);
    S_CLEAR_L((void *)&msg->msgi, 16);

    if ((connector_index) && (ne.ConnectChange == 1)) {
        cci->ConnectorChangeIndicator = connector_index;
        invalid = 0;
    }

    if (ack && ne.CommandCompleted) {
        invalid = 0;
        cci->CommandCompletedIndicator = 1;
    }

    if (invalid)
        return UCSI_COMMAND_FAIL;

    if (ne.CommandCompleted) {

        memcpy((XBYTE *)&xUCSI_DS_CCI0, (XBYTE *)cci, 4);
        ppm_alert();
    }

    return UCSI_COMMAND_SUCC;
}


/*
 * @desc   : UCSI PPM handler, EC should call this handler after received
 *           event from OPM/BIOS
 * @param  : none
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
void checking_opm_command(void)
{
    if (ucsi_received_command == 1) {
        ucsi_received_command = 0;

        RamDebug(0xAA);
        
        if (current_pending_command == 0) {
            memcpy((void *)&msg->CONTROL, (XBYTE *)&xUCSI_COMMAND, 8);
			RamDebug(current_pending_command);
            #ifdef ANXDEBUG
            ucsi_debug("%s:%d received command %.bx.\n", __func__, __LINE__, current_pending_command);
            #endif
            current_pending_command_timeout = 250;
            ucsi_errno = 0;
            S_CLEAR_L((void *)&msg->msgi, 16);
        } else {
            #ifdef ANXDEBUG
            ucsi_debug("%s:%d, received command %.bx at busy state.\n", __func__, __LINE__, xUCSI_COMMAND);
            #endif
            ppm_busy_process(xUCSI_COMMAND);
    }


    }

}

#ifdef ANXDEBUG
#define assert(x)   {\
    if (!x) {\
        ucsi_debug("%s:%d assert failed: %s.\n", __FILE__, __LINE__, #x);\
        while (1) ;\
    }\
}

void ucsi_structure_size_assert(void)
{
    assert(sizeof(struct bios_share_msg) == 48);
    assert(sizeof(struct cci_status) == 4);
    assert(sizeof(struct control_header) == 2);

    /* Checking all command structure */
    assert(sizeof(struct reset) == 8);
    assert(sizeof(struct cancel) == 8);
    assert(sizeof(struct connector_reset) == 8);
    assert(sizeof(struct ack_cc_ci) == 8);
    assert(sizeof(struct set_notification_enable) == 8);
    assert(sizeof(struct get_capability) == 8);
    assert(sizeof(struct get_connector_capability) == 8);
    assert(sizeof(struct set_uom) == 8);
    assert(sizeof(struct set_uor) == 8);
    assert(sizeof(struct set_pdm) == 8);
    assert(sizeof(struct set_pdr) == 8);
    assert(sizeof(struct get_alternate_modes) == 8);
    assert(sizeof(struct get_cam_supported) == 8);
    assert(sizeof(struct get_current_cam) == 8);
    assert(sizeof(struct set_new_cam) == 8);
    assert(sizeof(struct get_pdos) == 8);
    assert(sizeof(struct get_cable_property) == 8);
    assert(sizeof(struct get_connector_status) == 8);
    assert(sizeof(struct get_error_status) == 8);

    /* Checking specific data structure */
    assert(sizeof(struct capability_data) == 16);
    assert(sizeof(struct connector_capability_data) == 2);
    assert(sizeof(struct alt_modes_data) == 12);
    assert(sizeof(struct cable_property_data) == 5);
    assert(sizeof(struct connector_status_data) == 9);
    assert(sizeof(struct error_status_data) == 16);

    return;
}

#endif

/*
 * @desc   : EC should call this function to initial UCSI framework
 * @param  :
 *           msg -> shared message buffer between PPM and BIOS
 *           notify_callbak -> UCSI use this callbak function to notify BIOS
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
u8 ucsi_init()
{
#ifdef ANXDEBUG
    ucsi_debug("%s:%d ucsi initialization, ", __func__, __LINE__);
    ucsi_debug("ANX ucsi version is %.bx.%.bx.%.bx\n",
        ANX_UCSI_VERSION_MAJOR,
        ANX_UCSI_VERSION_MINOR,
        ANX_UCSI_VERSION_REV);
    ucsi_structure_size_assert();
#endif

//EVB only
#ifdef ITE_EC_EVB_PORT1_TEST
#message "ANX INIT on EVB"

    /*Initial GPIO, H6->Liberty 0, H7->Liberty 1*/
    GPCRH5 = INPUT;
    GPCRH6 = INPUT;
    GPCRH7 = MaxD;

#else
#message "ANX INIT on MRD"

#endif
    timer1msCount = 0;
	
    request_src_caps_flag[0] = 1;
    request_src_caps_flag[1] = 1;
    ucsi_ppm_state = PPM_IDLE_ND;
    connector_change_need_ack = 0;
    command_complete_need_ack = 0;
    need_ack_command = 0;
    current_pending_command = 0;
    self_charging_error = 0;
    supply_1500ma_flag = 0;
    
    msg->version = UCSI_COMPLIANT_VERSION;
    #ifdef EC_ANX_BIG_ENDIAN
    SWAP_WORD(&msg->version);
    memcpy((XBYTE *)&xUCSI_DS_VERSION, &msg->version, 2);
    #endif

	RamDebug(0xE6);
    ppm_init();
	RamDebug(0xE7);
    cancel_all_timer();	
	RamDebug(0xEB);
    anx7447_ucsi_init();	
	RamDebug(0xEC);
    anx_dead_battery_checking_1();    
	RamDebug(0xED);
    GOTO_STATE(PPM_IDLE_ND);	
	RamDebug(0xEE);
#ifdef ANXDEBUG
    ucsi_debug("\n\n");
#endif


#ifndef ITE_EC_EVB_PORT1_TEST
    //save UCSI version to memory
    xEC_UCSIMajorVersion = ANX_UCSI_VERSION_MAJOR;
    xEC_UCSIMinorVersion = ANX_UCSI_VERSION_MINOR;
    xEC_UCSIRevVersion = ANX_UCSI_VERSION_REV;
#endif

    return UCSI_COMMAND_SUCC;
	RamDebug(0xEA);
}


/*
 * @desc   : ANX context data structure initialization
 * @param  :
 *           param : anx platform context parameters
 * @return : none
 */
void anx_initial_context(u8 port_index)
{
    context[port_index].ucsi_connector_partner_type = 0;
    context[port_index].peer_svid_length = 0;
    context[port_index].downstream_pd_cap = 0;
    context[port_index].ucsi_partner_rdo = 0;
    context[port_index].power_op_mode = 0;
    context[port_index].anx_current_in_50ma = 0;
    context[port_index].anx_voltage_in_100mv = 0;
    context[port_index].power_role_swap_flag = 0;
    context[port_index].partner_pdo_length = 0;
    context[port_index].partner_pdo_sink_or_source = 0;
    context[port_index].sys_sta_bak = 0;
    context[port_index].anx_vbus_status = 0;
    context[port_index].pbuf_rx_front = 0;
    context[port_index].pbuf_tx_rear = 0;
    context[port_index].anx_power_role = 1;
	context[port_index].anx_data_role = 0;
    context[port_index].anx_initial_status = 0;
    //context->hard_reset_done = 0;
    //context->soft_reset_done = 0;
    context[port_index].enable_report_power_on = 0;
    S_CLEAR(&context[port_index].cv, struct cable_vdo);
    context[port_index].ct = CABLE_TYPE_A;
    //context[port_index].connector_index = 0;
    self_charging_error = 0;
    context[port_index].trysnk_enable = 0;
    context[port_index].trysrc_enable = 0;
    context[port_index].ConnectorPartner = CP_USB;
    context[port_index].BatteryCharging = 0;
    context[port_index].data_operation_mode_uor = OP_DRP;
    context[port_index].power_operation_mode_pdr = PW_DRP;
    context[port_index].hpd_status = 0;
    S_CLEAR(&context[port_index].csc.csc, struct connector_status_change);
    if (context[port_index].pd_capability & PD_HAS_REDRIVER)
        context[port_index].mux_mode = NO_CONNECT;
    if (port_index == supply_1500ma_port && supply_1500ma_flag == 1) {
        supply_1500ma_port = 0;
        supply_1500ma_flag = 0;
    }
    
}

static void pd_initialization(void)
{
    for (i = 0; i < PD_MAX_INSTANCE; i++) {
        if (context[i].anx_power_status == 1 && context[i].anx_initial_status) {
			RamDebug(0x67);
            port_id = i;
            send_initialized_setting();
        }
    }
}

void ucsi_async_notify_raise_up(u8 ms)
{
    switch (ucsi_ppm_state) {
    case PPM_IDLE_ND:
        break;
    case PPM_IDLE_NE:
        ucsi_async_checking_timer = ms;
        break;
    default:
        ucsi_async_restore = ms;
        break;
    }
}

/*
 * @desc   : UCSI PPM handler, EC should call this handler after received
 *           event from OPM/BIOS
 * @param  : none
 * @return :
 *           UCSI_COMMAND_SUCC : success
 *           UCSI_COMMAND_FAIL : fail
 */
void ucsi_opm_handler(void)
{
    switch (ucsi_ppm_state) {
        case PPM_IDLE_ND:
            ppm_nd_process();
            break;
        case PPM_IDLE_NE:
            ppm_ne_process();
            break;
        case PPM_PC:
            ppm_pc_process();
            break;
        case PPM_BUSY:
            break;
        case PPM_WCCA:
            ppm_wcca_process();
            break;
    }
}

void ANX_HOOK_10ms(void)
{
    pd_initialization();
    checking_opm_command();
    ucsi_opm_handler();
}
void ANX_HOOK_1ms(void)
{
    timer1msCount++;
#if Two_PD_Chip
    port_id = 1;	
#ifdef ITE_EC_EVB_PORT1_TEST
#message "ANX INTR on EVB Configuration"
    if ((GPDRH & BIT6) == 0)
#else
    if (IS_USBPD2_INT_N_LO()) 
#endif
    {
		RamDebug(0xCC);
        if (context[port_id].error_count < 100)
        {
			RamDebug(0xCF);
            anx_alert_message_isr();
            ++context[port_id].error_count;
        } else {
            if (timer1msCount == 200) {
				RamDebug(0xFD);
            }
        }
    }
    else
    {
        context[port_id].error_count = 0;
    }
#endif
    port_id = 0;
#ifdef ITE_EC_EVB_PORT1_TEST
#message "ANX INTR on EVB Configuration"
    if ((GPDRH & BIT5) == 0)
#else
    if (IS_USBPD1_INT_N_LO()) 
#endif
    {
		//RamDebug(0xBB);
        if (context[port_id].error_count < 100)
        {
			RamDebug(0xBC);
            anx_alert_message_isr();
			RamDebug(0xBD);
            ++context[port_id].error_count;
        }
        else {
            if (timer1msCount == 200){
            }
			//RamDebug(0xED);
        }
    }
    else
    {
        context[port_id].error_count = 0;
    }

	
}

/*
 * @desc   : anx_request_voltage_in_100mv, EC should call this to check the
 *           voltage while Type-C connector work as power sink.
 * @param  : port_id -> port id number (0 or 1)
 * @return :
 *           0 : Type-C connector work as power source.
 *           other : voltage, unit is 100mv.
 */
u8 anx_request_voltage_in_100mv(u8 port_id)
{
    u8 vol = 0;
	//anx_dead_battery_checking();
    if (self_charging_error == 1)
        return vol;
    if (context[port_id].anx_power_status == 0 || context[port_id].anx_power_role == 1)
        return 0;

    vol = (context[port_id].anx_voltage_in_100mv == 0) ? 50 : context[port_id].anx_voltage_in_100mv;
    #ifdef ANXDEBUG
    //ucsi_debug("     +++++++++  voltage : %x, in 100mv   +++++++++\n", (u16)vol);
    #endif
    return vol;
}

/*
 * @desc   : anx_request_current_in_50ma, EC should call this to check the
 *           current while Type-C connector work as power sink.
 * @param  : port_id -> port id number (0 or 1)
 * @return :
 *           0 : Type-C connector work as power source.
 *           other : current, unit is 50ma.
 */
u8 anx_request_current_in_50ma(u8 port_id)
{
    u8 cur = 0;
    //anx_dead_battery_checking();
    if (self_charging_error == 1)
        return cur;
    if (context[port_id].anx_power_status == 0 || context[port_id].anx_power_role == 1)
        return cur;

        cur = context[port_id].anx_current_in_50ma;
    #ifdef ANXDEBUG
    //ucsi_debug("     +++++++++  current : %x, in 50ma   +++++++++\n", (u16)cur);
    #endif

    return cur;
}

/*
 * @desc   : anx_connector_power_status, EC should call this to check whether
 *           the PD chip is power provider or consumer
 * @param  : port_id -> port id number (0 or 1)
 * @return :
 *           0 : power provider or PD standby
 *           other : power consumer
 */
u8 anx_connector_power_status(u8 port_id)
{
	//anx_dead_battery_checking();
    return (context[port_id].anx_power_role == 0);
}


void anx_ucsi_command_received(void)
{
    ucsi_received_command = 1;
}

/* LBT-540 */
/*
 * @desc   : anx_ucsi_cc_status_event, EC may need add cc status event processing
 * @param  : port_id -> port id number (0 or 1)
 *                 cc_status      ->  cc status
 * @return : none
 */
void anx_ucsi_cc_status_event(u8 port_id, u8 cc_status)
{
    if (cc_status == 0)
        return;

    //customer add interrupt processing.
    //cc direction -> context[port_id].cc_orientation
    //  value :   CC1_CONNECTED or CC2_CONNECTED
}

/*
 * @desc   : anx_ucsi_pin_assignment_event, EC may add pin assignment event processing
 * @param  : port_id -> port id number (0 or 1)
 *                 pin_assignment      ->  pin assignment
                                     SELECT_PIN_ASSIGMENT_C, SELECT_PIN_ASSIGMENT_D
                                     SELECT_PIN_ASSIGMENT_E,  SELECT_PIN_ASSIGMENT_U
 * @return : none
 */
void anx_ucsi_pin_assignment_event(u8 port_id, u8 pin_assignment)
{
    //customer add interrupt processing.
    switch (pin_assignment) {
    case SELECT_PIN_ASSIGMENT_U:
        //default 2 line USB 3.1
        break;
    case SELECT_PIN_ASSIGMENT_C:
    case SELECT_PIN_ASSIGMENT_E:
        // 4 line DP
        break;
    case SELECT_PIN_ASSIGMENT_D:
        // 2 line DP, 2 line USB
        break;
    }
}

/*
 * @desc   : anx_ucsi_hpd_event, EC may add HPD event processing
 * @param  : port_id -> port id number (0 or 1)
 *                 hpd      -> 1:HPD high, 0 HPD low
 * @return : none
 */
void anx_ucsi_hpd_event(u8 port_id, u8 hpd)
{
    //customer may add event processing.
}

/*
 * @desc   : anx_ucsi_detach_event, EC may add detach event processing
 * @param  : port_id -> port id number (0 or 1)
 * @return : none
 */
void anx_ucsi_detach_event(u8 port_id)
{
    // customer may add detach event processing, include reset MUX/HPD
}

/*
 * @desc   : anx_ucsi_renotify_OPM, EC may call it at reboot stage
 * @param  : port_id -> port id number (0 or 1)
 * @return : none
 */
void anx_ucsi_renotify_OPM(u8 port_id)
{
    // customer may add detach event processing, include reset MUX/HPD
    if (context[port_id].anx_power_status == 1) {
        #ifdef ANXDEBUG
        ucsi_debug("re-notify OPM at reboot stage.\n");
        #endif
        context[port_id].csc.csc.Connect = 1;
        context[port_id].csc.csc.ConnectorPartner = 1;
        context[port_id].csc.csc.BatteryChargingStatus = 1;
        if (context[port_id].ucsi_partner_rdo) {
            #ifdef ReportNegotiatedPowerLevel
            context[port_id].csc.csc.NegotiatedPowerLevel = 1;
            #endif
            #ifdef ReportSupportedProviderCap
            context[port_id].csc.csc.SupportedProviderCap = 1;
            #endif
        }
        ucsi_async_notify_raise_up(250);
    }
}


