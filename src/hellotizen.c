//#include "hellotizen.h"
#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <sensor.h>
#include <dlog.h>
#include <time.h>
#include <glib.h>

#include "hellotizen.h"

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "HELLOTIZEN"
#ifdef  LOG_TAG_ACCELEROMETER
#undef  LOG_TAG_ACCELEROMETER
#endif
#define LOG_TAG_ACCELEROMETER "ACCELEROMETER_TEST"

#if !defined(PACKAGE)
#define PACKAGE "org.example.hellotizen"
#endif
#define BUFLEN 200
#define MIN_INTERVAL_S 500
#define DEFAULT_MEASURE_DURATION 15
#define HRM_PREFIX "HRM_START"
#define ACCELEROMETER_PREFIX "ACC_START"

/*=============================UI EVAS START HERE================================*/
/* An Evas object is the most basic visual entity used in Evas.
 * Everything, be it a single line or a complex list of UI components, is an Evas object. */

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *navi;

} appdata_s;

Evas_Object *startHRM;
Evas_Object *stopHRM;

Evas_Object *HeartRate_event_label;
Evas_Object *Accuracy_event_label;
Evas_Object *Accuracy_event_label_data;

sensor_listener_h listenerHRM;
sensor_listener_h listenerACC;

Evas_Object *new_button(appdata_s *ad, Evas_Object *parrent, char *name,
		void *action) {

	Evas_Object *bt = elm_button_add(parrent);
	elm_object_text_set(bt, name);
	evas_object_smart_callback_add(bt, "clicked", (Evas_Smart_Cb) action, ad);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(parrent, bt);
	evas_object_show(bt);
	return bt;
}

/* Registering a Callback for sensor event  */
void on_sensor_event_HRM(sensor_h sensor, sensor_event_s *event,
		void *user_data) {

	//system time not needed anymore, sensor returns epoch!
	/*
	 time_t raw_time;
	 struct tm* time_info;
	 time(&raw_time);
	 time_info = localtime(&raw_time);
	 */

	// Select a specific sensor with a sensor handle
	sensor_type_e type;
	sensor_get_type(sensor, &type);

	switch (type) {

	case SENSOR_HRM:

		dlog_print(DLOG_INFO, LOG_TAG, "Time: %llu, HR: %d, Accuracy: %d",
				event->timestamp, event->values[0], event->accuracy);
		char values[150];
		char heart_rate[30];
		char accuracy[10];
		char timestamp[40];
		char label_accuracy[15];
		char hrm_prefix[] = HRM_PREFIX;
		sprintf(heart_rate, "%f", event->values[0]);
		sprintf(accuracy, "%d", event->accuracy);
		sprintf(timestamp, "%llu", event->timestamp);
		//sprintf(hour, "%d", time_info->tm_hour);
		//sprintf(minute, "%d", time_info->tm_min);
		//sprintf(second, "%d", time_info->tm_sec);

		strcat(values, hrm_prefix);
		strcat(values, ":");
		strcat(values, heart_rate);
		strcat(values, ":");
		strcat(values, accuracy);
		strcat(values, ":");
		strcat(values, timestamp);
		strcat(values, ":");
		strcat(values, "END_HRM");

		gboolean is_secured = false;
		int length = strlen(values);

		elm_object_text_set(HeartRate_event_label, heart_rate);
		strcat(label_accuracy, "Accuracy: ");
		strcat(label_accuracy, accuracy);
		elm_object_text_set(Accuracy_event_label, label_accuracy);
		mex_send(values, length, is_secured);

		break;
	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "Not an HRM event");
	}

}
void on_sensor_event_ACCELEROMETER(sensor_h sensor, sensor_event_s *event,
		void *user_data) {

	// Select a specific sensor with a sensor handle
	sensor_type_e type;
	sensor_get_type(sensor, &type);
	switch (type) {

	//print mesurement to console
	case SENSOR_ACCELEROMETER:

		//3 Cartesian axis values and a timestamp X,Y,Z
		dlog_print(DLOG_INFO, LOG_TAG_ACCELEROMETER,
				"X: %f, Y: %f, Z: %f, timestamp: %llu", event->values[0],
				event->values[1], event->values[2], event->timestamp);

		char acc_values[200];
		char acc_value_X[25];
		char acc_value_Y[25];
		char acc_value_Z[25];
		char acc_value_epoch[25];
		char acc_accuracy[5];
		char acc_prefix[] = ACCELEROMETER_PREFIX;
		sprintf(acc_value_X, "%f", event->values[0]);
		sprintf(acc_value_Y, "%f", event->values[1]);
		sprintf(acc_value_Z, "%f", event->values[2]);
		sprintf(acc_value_epoch, "%llu", event->timestamp);
		sprintf(acc_accuracy, "%d", event->accuracy);

		//snprintf(buf, sizeof buf, "%s%s%s%s", values, acc, timestamp, str4);
		strcat(acc_values, acc_prefix);
		strcat(acc_values, ":");
		strcat(acc_values, acc_value_X);
		strcat(acc_values, ":");
		strcat(acc_values, acc_value_Y);
		strcat(acc_values, ":");
		strcat(acc_values, acc_value_Z);
		strcat(acc_values, ":");
		strcat(acc_values, acc_accuracy);
		strcat(acc_values, ":");
		strcat(acc_values, acc_value_epoch);
		strcat(acc_values, ":");

		strcat(acc_values, "END_ACC");

		gboolean is_secured = false;
		int length = strlen(acc_values);

		mex_send(acc_values, length, is_secured);

		break;
	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "Not an ACCELEROMETER event");
	}

}
void _sensor_accuracy_changed_cb(sensor_h sensor, unsigned long long timestamp,
		sensor_data_accuracy_e accuracy, void *data) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor accuracy callback invoked");
	sensor_event_s event;

	int error = sensor_listener_read_data(listenerHRM, &event);
	dlog_print(DLOG_DEBUG, LOG_TAG,
			"FROM CALLBACK sensor accuracy changed at: %lu accuracy is: %d",
			timestamp, event.accuracy);
	char accuracy1[100];
	sprintf(accuracy1, "%d", event.accuracy);
	//elm_object_text_set(Accuracy_event_label, accuracy1);
}

void stop_sensor_ALL() {
	int error = sensor_listener_unset_event_cb(listenerHRM);
	error = sensor_listener_unset_event_cb(listenerACC);

	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_unset_event_cb error: %d", error);
	}

	error = sensor_listener_stop(listenerHRM);
	error = sensor_listener_stop(listenerACC);

	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_stop error: %d",
				error);
	}

	error = sensor_destroy_listener(listenerHRM);
	error = sensor_destroy_listener(listenerACC);

	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_destroy_listener error: %d",
				error);
	}
	elm_object_disabled_set(startHRM, EINA_FALSE);
	elm_object_disabled_set(stopHRM, EINA_TRUE);
}

void _sensor_stop_cb_HRM(void *data, Evas_Object *obj, void *event_info) {
//void _sensor_stop_cb() {
	int error = sensor_listener_unset_event_cb(listenerHRM);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_unset_event_cb error: %d", error);
	}

	error = sensor_listener_stop(listenerHRM);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_stop error: %d",
				error);
	}

	error = sensor_destroy_listener(listenerHRM);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_destroy_listener error: %d",
				error);
	}

	elm_object_disabled_set(startHRM, EINA_FALSE);
	elm_object_disabled_set(stopHRM, EINA_TRUE);
}

void _sensor_start_cb_HRM() {

	time_t raw_time;
	struct tm* time_info;

	void *user_data = NULL;
	char out[100];

	bool supported = false;

	/* Define sensor type SENSOR_HRM */
	sensor_type_e type = SENSOR_HRM;

	/* Create Handle for sensor */
	sensor_h sensor;

	/* Check if sensor is supported */
	int error = sensor_is_supported(type, &supported);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_is_supported function error: %d", error);
		return;
	}
	if (supported) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "HRM is%s supported",
				supported ? "" : " not");
		//TODO [DEL] sprintf(out, "HRM is%s supported", supported ? "" : " not");
		//TODO [DEL] elm_object_text_set(event_label, out);
	}

	/* Count of Heart Rate Sensors */
	int count = 0;

	/* list of Heart Rate Sensors */
	sensor_h *list;

	/* Get all available sensors of type SENSOR_HRM, add to list */
	error = sensor_get_sensor_list(type, &list, &count);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_sensor_list error: %d",
				error);
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG, "Number of HRM sensors: %d", count);
		free(list);
	}

	/* Get the Default Heart Rate Sensor (sensor of type) */
	error = sensor_get_default_sensor(type, &sensor);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d",
				error);
		return;
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG,
				"sensor_get_default_sensor, HRM Sensor Found!");
	}

	/* Create a sensor listener */
	error = sensor_create_listener(sensor, &listenerHRM);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d",
				error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_create_listener");

	/* Register a callback to be invoked when sensor events are delivered via a sensor listener [above]  */
	error = sensor_listener_set_event_cb(listenerHRM, MIN_INTERVAL_S,
			on_sensor_event_HRM, user_data);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_set_event_cb error: %d", error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_event_cb");

	/* set the interval for HRM in milliseconds. 100-1000ms */
	error = sensor_listener_set_interval(listenerHRM, MIN_INTERVAL_S);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_set_interval error: %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_intervals");

	// Registering the Accuracy Changed Callback
	error = sensor_listener_set_accuracy_cb(listenerHRM,
			_sensor_accuracy_changed_cb, user_data);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_set_accuracy_cb error: %d", error);
		return;

	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_accuracy_cb");

	/* Changes the power-saving behavior of a sensor listener. [ALWAYS ON] */
	error = sensor_listener_set_option(listenerHRM, SENSOR_OPTION_ALWAYS_ON);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_option");

	/* START the sensor listener */
	error = sensor_listener_start(listenerHRM);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_start");

	/* Read sensor data (from started listener). */
	sensor_event_s event;
	error = sensor_listener_read_data(listenerHRM, &event);
	if (error != SENSOR_ERROR_NONE) {

		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_read_data error: %d",
				error);
		return;
	}

	switch (type) {

	case SENSOR_HRM:
		//print mesurement to console
		dlog_print(DLOG_INFO, LOG_TAG, "%f", event.values[0]);

		//insert timestamp (logging)
		time(&raw_time);
		time_info = localtime(&raw_time);

		break;
	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "Not an HRM event");
	}
	//Set the button states
	elm_object_disabled_set(startHRM, EINA_TRUE);
	elm_object_disabled_set(stopHRM, EINA_TRUE);

}

void _sensor_start_cb_ACCELEROMETER() {

	void *user_data = NULL;

	bool supported = false;

	/* Define sensor type SENSOR_HRM */
	sensor_type_e type = SENSOR_ACCELEROMETER;

	/* Create Handle for sensor */
	sensor_h sensor;

	/* Check if sensor is supported */
	int error = sensor_is_supported(type, &supported);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_is_supported function error: %d", error);
		return;
	}
	if (supported) {
		dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER,
				"ACCELEROMETER is%s supported", supported ? "" : " not");
	}

	/* Count of ACCELEROMETER */
	int count = 0;

	/* list of ACCELEROMETER*/
	sensor_h *list;

	/* Get all available sensors of type ACCELEROMETER, add to list */
	error = sensor_get_sensor_list(type, &list, &count);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_get_sensor_list error: %d", error);
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER,
				"Number of ACCELEROMETER sensors: %d", count);
		free(list);
	}

	/* Get the Default ACCELEROMETER (sensor of type) */
	error = sensor_get_default_sensor(type, &sensor);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_get_default_sensor error: %d", error);
		return;
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER,
				"sensor_get_default_sensor, ACCELEROMETER Sensor Found!");
	}

	/* Create a sensor listener */
	error = sensor_create_listener(sensor, &listenerACC);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_create_listener error: %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER, "sensor_create_listener");

	/* Register a callback to be invoked when sensor events are delivered via a sensor listener [above]  */
	error = sensor_listener_set_event_cb(listenerACC, MIN_INTERVAL_S,
			on_sensor_event_ACCELEROMETER, user_data);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_listener_set_event_cb error: %d", error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER,
			"sensor_listener_set_event_cb");

	/* set the interval for ACCELEROMETER in milliseconds. 100-1000ms */
	error = sensor_listener_set_interval(listenerACC, MIN_INTERVAL_S);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_listener_set_interval error: %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER,
			"sensor_listener_set_intervals");

	/*
	 // Registering the Accuracy Changed Callback
	 error = sensor_listener_set_accuracy_cb(listener,
	 _sensor_accuracy_changed_cb, user_data);
	 if (error != SENSOR_ERROR_NONE) {
	 dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
	 "sensor_listener_set_accuracy_cb error: %d", error);
	 return;

	 }
	 dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER,
	 "sensor_listener_set_accuracy_cb");
	 */

	/* Changes the power-saving behavior of a sensor listener. [ALWAYS ON] */
	error = sensor_listener_set_option(listenerACC, SENSOR_OPTION_ALWAYS_ON);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_listener_set_option error: %d", error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER, "sensor_listener_set_option");

	/* START the sensor listener */
	error = sensor_listener_start(listenerACC);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
				"sensor_listener_start error: %d", error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG_ACCELEROMETER, "sensor_listener_start");

	elm_object_disabled_set(startHRM, EINA_TRUE);
	elm_object_disabled_set(stopHRM, EINA_TRUE);

	/* Read sensor data (from started listener).
	 sensor_event_s event;
	 error = sensor_listener_read_data(listener, &event);
	 if (error != SENSOR_ERROR_NONE) {

	 dlog_print(DLOG_ERROR, LOG_TAG_ACCELEROMETER,
	 "sensor_listener_read_data error: %d", error);
	 return;
	 }*/
}

/* registering callback method event to call
 ui_app_exit() when object is deleted */
static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info) {
	appdata_s *ad = data;

	/* Let window go to hide state. */
	elm_win_lower(ad->win);
	ui_app_exit();
}

void update_ui(char *data) {
	/*
	 * If desired this function can be used to trigger
	 * a ui change when message is received from smartphone
	 */
	//elm_object_text_set(object->naviframe, data);
}

Eina_Bool _pop_cb(void *data, Elm_Object_Item *item) {
	elm_win_lower(((appdata_s *) data)->win);
	return EINA_FALSE;
}

void _create_new_cd_display(appdata_s *ad, char *name, void *cb) {

	// Create main box
	Evas_Object *box = elm_box_add(ad->conform);
	elm_object_content_set(ad->conform, box);
	elm_box_horizontal_set(box, EINA_FALSE);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);
//todo change to _sensor_start_cb_HRM
	startHRM = new_button(ad, box, "Start", _sensor_start_cb_ACCELEROMETER);

	HeartRate_event_label = elm_label_add(box);
	Accuracy_event_label = elm_label_add(box);
	Accuracy_event_label_data = elm_label_add(box);

	elm_object_text_set(HeartRate_event_label, "click start to measure");
	elm_object_text_set(Accuracy_event_label, "Accuracy:");
	elm_object_text_set(Accuracy_event_label_data, "123");

	elm_box_pack_end(box, HeartRate_event_label);
	elm_box_pack_end(box, Accuracy_event_label);

	evas_object_size_hint_align_set(Accuracy_event_label, 0.1, -1.0);

	evas_object_show(HeartRate_event_label);
	evas_object_show(Accuracy_event_label);

	stopHRM = new_button(ad, box, "Stop", _sensor_stop_cb_HRM);

}

/** Window --> Conformant --> BaseLayout **/
static void create_base_gui(appdata_s *ad) {

	/* set up policy to exit when last window is closed */
	/* Using the elm_policy_set() function, make the app close when the last
	 window closes. */
	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	/* Window */

	/* elm_win_util_standard_add(char * NAME, char * TITLE) */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);

	/* Set the Window (win) autodelete state (autodel)
	 * the window will automatically delete itself when closed */
	elm_win_autodel_set(ad->win, EINA_TRUE);

	/* Qerry windiw manager if rotation is supported */
	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };

		/* set the array of available window rotations */
		elm_win_wm_rotation_available_rotations_set(ad->win,
				(const int *) (&rots), 4);
	}
	/* register a callback function to the specified object EVENT (win) [DELETE] */
	evas_object_smart_callback_add(ad->win, "delete,request",
			win_delete_request_cb, NULL);

	/* register a callback function to the specified object EVENT (win) [HARDWARE_BACK_BUTTON] */
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb,
			ad);

	/* Conformant */
	/*Add a conformant widget to the given parent Elementary (container) object (win)*/
	ad->conform = elm_conformant_add(ad->win);

	/* Set the indicator mode of the window mode */
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);

	/*[NOT A SIZE ENFORCEMENT] - hint on how a container object should resize a given child within
	 * its area, primitive EVAS_HINT_EXPAND */
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);

	/* adds subobj as a resize object of the window obj */
	elm_win_resize_object_add(ad->win, ad->conform);

	/* Make the conformant visible */
	evas_object_show(ad->conform);

	// Create a naviframe
	ad->navi = elm_naviframe_add(ad->conform);
	evas_object_size_hint_align_set(ad->navi, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->navi, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->navi);
	evas_object_show(ad->navi);

	//Lets call new button, testing - bug, replaces label!
	Evas_Object *box = elm_box_add(ad->conform);
	elm_object_content_set(ad->conform, box);

	/* Create the buttons and the labels, register callback to pop window and put behind everything*/
	_create_new_cd_display(ad, "MainBox", _pop_cb);

	/* Make the window visible after base GUI is set up */
	evas_object_show(ad->win);
}
/*=============================UI EVAS END HERE================================*/

static bool app_create(void *data) {

	create_base_gui((appdata_s *) data);
	initialize_sap();

	return true;
}

static void app_terminate(void *data) {
	/* Release all resources for sensor. */
	int error = sensor_listener_stop(listenerACC);
	error = sensor_destroy_listener(listenerACC);

	error = sensor_listener_stop(listenerHRM);
	error = sensor_destroy_listener(listenerHRM);
}

int main(int argc, char *argv[]) {

	appdata_s ad;
	memset(&ad, 0x00, sizeof(appdata_s));
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
