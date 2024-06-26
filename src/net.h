/*
 * Copyright (c) 2020  teakhanirons
 * Modded By BenMitnicK
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

extern char vita_ip[16];
extern unsigned short int vita_port;

void net_start();
void net_end();
int net_thread(unsigned int args, void *argp);
void do_net_connected();
void sendNotification(const char *text, ...);
void utf8_to_utf16(const uint8_t *src, uint16_t *dst);

#endif
