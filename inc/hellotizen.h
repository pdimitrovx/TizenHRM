#ifndef __hellotizen_H__
#define __hellotizen_H__


#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#define NUM_OF_ITEMS 5

void initialize_sap();
int read_sensor_data();
void update_ui(char *data);

#if !defined(PACKAGE)
#define PACKAGE "org.example.hellotizen"
#endif


#endif /* __hellotizen_H__ */





