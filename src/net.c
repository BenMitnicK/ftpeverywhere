#include "net.h"
#include "ftpvita.h"

#include <vitasdk.h>
#include <stdio.h>
#include <stdbool.h>

#include <quickmenureborn/qm_reborn.h>

#define print printf

#define BUTTON_REF_ID "qm_reborn_sample_button"

extern int run, s_mesg;
extern int start_thread(void);
extern void checkWifiPlane();

char vita_ip[16];
unsigned short int vita_port;

SceUID net_thid;
static int netctl_cb_id;

void utf8_to_utf16(const uint8_t *src, uint16_t *dst) {
  int i;
  for (i = 0; src[i];) {
    if ((src[i] & 0xE0) == 0xE0) {
      *(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
      i += 3;
    } else if ((src[i] & 0xC0) == 0xC0) {
      *(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
      i += 2;
    } else {
      *(dst++) = src[i];
      i += 1;
    }
  }

  *dst = '\0';
}

void sendNotification(const char *text, ...)
{
	SceNotificationUtilProgressInitParam param;

	char buf[SCE_NOTIFICATIONUTIL_TEXT_MAX * 2];
	va_list argptr;
	va_start(argptr, text);
	sceClibVsnprintf(buf, sizeof(buf), text, argptr);
	va_end(argptr);

	sceClibMemset(&param, 0, sizeof(SceNotificationUtilProgressInitParam));

	utf8_to_utf16((uint8_t *)buf, param.notificationText);

	sceNotificationUtilSendNotification(&param);
}

void net_start() {
    net_thid = sceKernelCreateThread("ftpeverywhere_thread", net_thread, 0x40, 0x10000, 0, 0, NULL);
    sceKernelStartThread(net_thid, 0, NULL);
}

void net_end() {
    sceNetCtlInetUnregisterCallback(netctl_cb_id);
    ftpvita_fini();
}

void do_net_connected() {

    print("ftpeverywhere connected\n");

    ftpvita_set_file_buf_size(512 * 1024);

    if (ftpvita_init(vita_ip, &vita_port) >= 0) {

		ftpvita_add_device("ux0:");
		ftpvita_add_device("ur0:");
		ftpvita_add_device("uma0:");
		ftpvita_add_device("imc0:");

		ftpvita_add_device("gro0:");
		ftpvita_add_device("grw0:");

		ftpvita_add_device("os0:");
		ftpvita_add_device("pd0:");
		ftpvita_add_device("sa0:");
		ftpvita_add_device("tm0:");
		ftpvita_add_device("ud0:");
		ftpvita_add_device("vd0:");
		ftpvita_add_device("vs0:");

		ftpvita_add_device("app0:");
		ftpvita_add_device("savedata0:");
		
		ftpvita_add_device("music0:");
		ftpvita_add_device("photo0:");
		run = 1;
		s_mesg = 0;
		start_thread();
    }
}

static void* netctl_cb(int event_type, void* arg) {
    print("netctl cb: %d\n", event_type);

    // TODO sceNetCtlInetGetResult

    if (event_type == 1 || event_type == 2 || event_type == 3) {

		run = 0;
		ftpvita_fini();
		
		//Update our widget with new size and text
		QuickMenuRebornSetWidgetSize(BUTTON_REF_ID, 300, 75, 0, 0);
		QuickMenuRebornSetWidgetColor(BUTTON_REF_ID, 1,1,1,1);
		QuickMenuRebornSetWidgetLabel(BUTTON_REF_ID, "FTP Disable");
    }

    return NULL;
}

int net_thread(unsigned int args, void* argp) {
    int ret;

    sceKernelDelayThread(3 * 1000 * 1000);
	
    ret = sceNetCtlInit();
    print("sceNetCtlInit: 0x%08X\n", ret);

    // if already connected to Wi-Fi
    int state;
    sceNetCtlInetGetState(&state);
    print("sceNetCtlInetGetState: 0x%08X\n", state);
    netctl_cb(state, NULL);

    // FIXME: Add a mutex here, network status might change right before the callback is registered

    ret = sceNetCtlInetRegisterCallback(netctl_cb, NULL, &netctl_cb_id);
    print("sceNetCtlInetRegisterCallback: 0x%08X\n", ret);

    while (1) {
        sceNetCtlCheckCallback();
        sceKernelDelayThread(1000 * 1000);
		
		if(!s_mesg)
			checkWifiPlane();
		
    }

    return 0;
}
