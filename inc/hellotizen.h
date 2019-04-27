#ifndef __hellotizen_H__
#define __hellotizen_H__


#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#define NUM_OF_ITEMS 5

void initialize_sap();
void update_ui(char *data);
void mex_send();
void _sensor_start_cb_HRM();
void start_streaming_data();
void stop_sensor_HRM();

#if !defined(PACKAGE)
#define PACKAGE "org.example.hellotizen"
#endif


#endif /* __hellotizen_H__ */





