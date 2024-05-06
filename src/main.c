/*
 * Copyright (c) 2020  teakhanirons
 * Modded By BenMitnicK
 */

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/threadmgr.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <taihen.h>
#define TAIPOOL_AS_STDLIB
#include "net.h"

#include <taipool.h>

#include <vitasdk.h>

#include "ftpvita.h"

#include <quickmenureborn/qm_reborn.h>

#define	USER_MESSAGE_THREAD_NAME	"user_message_sub_thr"
#define	USER_MESSAGE_THREAD_TIME	(2000000)	// 2 sec

extern SceUID net_thid;
extern int all_is_up;
extern int net_connected;

static SceUID	s_msgThreadId;

int run, s_mesg;

//define a refrence id for each our widgets and textures, make sure they're all unique, like the ones below. Doesn't need to be anything specific, just something unique
#define BUTTON_REF_ID "qm_reborn_sample_button"
#define CHECKBOX_REF_ID "qm_reborn_sample_checkbox"
#define PLANE_ID "qm_reborn_sample_plane"
#define TEXT_ID "qm_reborn_sample_text"
#define CHECKBOX_TEXT_ID "qm_reborn_sample_checkbox_text"
#define SLIDEBAR_ID "qm_reborn_sample_slidebar"
#define SEPARATOR_ID "qm_reborn_sample_separator"
#define TEX_PLANE_ID "qm_reborn_sample_plane_for_tex"
#define TEXTURE_REF_ID "qm_reborn_sample_texture"

#define BUTTON2_REF_ID "qm_reborn_sample_button"
#define CHECKBOX_NOTIF_ID "qm_reborn_sample_checkbox"
#define PLANE_NOTIF_ID "qm_reborn_sample_plane"
#define CHECKBOX_NOTIF_TEXT_ID "qm_reborn_sample_checkbox_text"

//Declare our boolean
bool ftpON = false;
bool resetOnExit = false;

//Set our current count
int count = 0;

typedef struct SceAppMgrEvent {
	int		event;			// Event ID 
	SceUID	appId;			// Application ID. Added when required by the event 
	char	param[56];		// Parameters to pass with the event 
} SceAppMgrEvent;

// appmgr
extern int sceAppMgrReceiveEvent(SceAppMgrEvent *appEvent);
SceAppMgrEvent appEvent;

void checkWifiPlane(){
	
	/* network check */

	int wifi, plane;
	
	sceRegMgrGetKeyInt("/CONFIG/NET", "wifi_flag", &wifi);
	sceRegMgrGetKeyInt("/CONFIG/SYSTEM", "flight_mode", &plane);
	
	if (!wifi || plane) {
		s_mesg = 1;
		sceSysmoduleLoadModule(SCE_SYSMODULE_INCOMING_DIALOG);

		sceIncomingDialogInitialize(0);
		SceIncomingDialogParam params;
		sceIncomingDialogParamInit(&params);

		utf8_to_utf16((uint8_t *)"ok", params.buttonRightText);
		utf8_to_utf16((uint8_t *)"Wi-Fi is Disabled Or System is in Airplane Mode.\nFTPEveryWhere will Start after WiFi is ON", params.dialogText);
		sceIncomingDialogOpen(&params);
		
		run = 0;
		ftpvita_fini();	
		start_thread();

	}
	
}

//Declare our function that will act as the callback for when our button is pressed, Format: BUTTON_HANDLER(name of function)
BUTTON_HANDLER(onPress)
{
		
	if(run){
		run = 0;
		ftpvita_fini();
		start_thread();
		//Update our widget with new size and text
		QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Disable");
	}else{
		checkWifiPlane();
		run = 1;
		do_net_connected();
		if(all_is_up)
		{
			//Update our widget with new size and text
			QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
			QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 0,1,0,1);
			QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Enable");
		}
	}
    
}

ONLOAD_HANDLER(OnButtonLoad)
{    
    if(run)
    {
        //Update our widget with new size and text
        QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 0,1,0,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Enable");
    }else{
		//Update our widget with new size and text
		QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Disable");
	}
}

BUTTON_HANDLER(OnToggleCheckBox)
{
    ftpON = QuickMenuRebornGetCheckboxValue(CHECKBOX_REF_ID);
}

SceInt32 thread_user_message(SceSize args, void *argc)
{
	int		res;
	char sendNotifText[256];

	if(run){
		sprintf(sendNotifText, "IP: %s\nPort: %i", vita_ip, vita_port);
		//Update our widget with new size and text
		QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 0,1,0,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Enable");
	}else{
		sprintf(sendNotifText, "FTPEveryWhere Disable");
		//Set new values
		QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Disable");
	}

	sceKernelDelayThread(USER_MESSAGE_THREAD_TIME);

	// Send Notification
	sendNotification(sendNotifText);

	res = sceKernelExitDeleteThread(0);
	return res;
}

int start_thread(void)
{
	int		res;

	res = sceKernelCreateThread(USER_MESSAGE_THREAD_NAME, thread_user_message, 0x40, 0x10000, 0, 0, NULL);
	if (res < SCE_OK) {
		printf("Error: sceKernelCreateThread %08x\n", res);
		return res;
	}
	s_msgThreadId = res;

	res = sceKernelStartThread(s_msgThreadId, 0, NULL);
	if (res < SCE_OK) {
		printf("Error: sceKernelStartThread %08x\n", res);
		return res;
	}

	return res;
}

void __unused _start() __attribute__((weak, alias("module_start")));
int __unused module_start(SceSize argc, const void* args) {

	QuickMenuRebornSeparator(SEPARATOR_ID, SCE_SEPARATOR_HEIGHT);

    //Get our checkboxes saved state
    int ret = QuickMenuRebornGetCheckboxValue(CHECKBOX_REF_ID);
    ftpON = ret == QMR_CONFIG_MGR_ERROR_NOT_EXIST ? false : ret;

    QuickMenuRebornRegisterWidget(TEXT_ID, NULL, text);
    QuickMenuRebornSetWidgetSize(TEXT_ID, SCE_PLANE_WIDTH, 50, 0, 0);
    QuickMenuRebornSetWidgetColor(TEXT_ID, 1,1,1,1);
    QuickMenuRebornSetWidgetPosition(TEXT_ID, 0, 0, 0, 0);
    QuickMenuRebornSetWidgetLabel(TEXT_ID, "File Transfer (FTP)");

    QuickMenuRebornRegisterWidget(PLANE_ID, NULL, plane);
    QuickMenuRebornSetWidgetSize(PLANE_ID, SCE_PLANE_WIDTH, 100, 0, 0);
    QuickMenuRebornSetWidgetColor(PLANE_ID, 1,1,1,0);

	QuickMenuRebornRegisterWidget(CHECKBOX_REF_ID, PLANE_ID, check_box);
    QuickMenuRebornSetWidgetSize(CHECKBOX_REF_ID, 48, 48, 0, 0);
    QuickMenuRebornSetWidgetColor(CHECKBOX_REF_ID, 1,1,1,1);
    QuickMenuRebornSetWidgetPosition(CHECKBOX_REF_ID, 350, 0, 0, 0);
    QuickMenuRebornAssignDefaultCheckBoxRecall(CHECKBOX_REF_ID);
    QuickMenuRebornAssignDefaultCheckBoxSave(CHECKBOX_REF_ID);
    QuickMenuRebornRegisterEventHanlder(CHECKBOX_REF_ID, QMR_BUTTON_RELEASE_ID, OnToggleCheckBox, NULL);

	QuickMenuRebornRegisterWidget(CHECKBOX_TEXT_ID, PLANE_ID, text);
    QuickMenuRebornSetWidgetColor(CHECKBOX_TEXT_ID, 1,1,1,1);
    QuickMenuRebornSetWidgetSize(CHECKBOX_TEXT_ID, 500, 75, 0, 0);
    QuickMenuRebornSetWidgetPosition(CHECKBOX_TEXT_ID, -200, 0, 0, 0);
    QuickMenuRebornSetWidgetLabel(CHECKBOX_TEXT_ID, "Enable/Disable Notifications");
	
	QuickMenuRebornRegisterWidget(BUTTON_REF_ID, NULL, button);
    QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
    //QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 1,1,1,1);
    QuickMenuRebornRegisterEventHanlder(BUTTON_REF_ID, QMR_BUTTON_RELEASE_ID, onPress, NULL);
    //QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Disable");
	//QuickMenuRebornSetWidgetPosition(BUTTON_REF_ID, -50, 0, 0, 0);
    QuickMenuRebornAssignOnLoadHandler(OnButtonLoad, BUTTON_REF_ID);
    
	taipool_init(1 * 1024 * 1024); // user plugins can't malloc without Libc which is not available in main
	run = 1;
	net_start();
		
	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	
#ifdef _DEBUG
	ftpvita_set_info_log_cb(sendNotification);
#endif
	ftpvita_set_notif_log_cb(sendNotification);
	
    return SCE_KERNEL_START_SUCCESS;
}

int __unused module_stop(SceSize argc, const void* args) {
    run = 0;
    sceKernelWaitThreadEnd(net_thid, NULL, NULL);

    if (all_is_up) {
        net_end();
    }

    taipool_term();

	//Remove our widgets from the list using our refrence ids, it will no longer be displayed
	QuickMenuRebornUnregisterWidget(BUTTON_REF_ID);
    QuickMenuRebornUnregisterWidget(CHECKBOX_REF_ID);
    QuickMenuRebornUnregisterWidget(TEXT_ID);
    QuickMenuRebornUnregisterWidget(CHECKBOX_TEXT_ID);
    QuickMenuRebornUnregisterWidget(PLANE_ID);
    QuickMenuRebornRemoveSeparator(SEPARATOR_ID); //Don't forget this!

    return SCE_KERNEL_STOP_SUCCESS;
}
