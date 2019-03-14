//#include "hellotizen.h"
#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <sensor.h>
#include <dlog.h>
#include <time.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "HELLOTIZEN"

#if !defined(PACKAGE)
#define PACKAGE "org.example.hellotizen"
#endif
#define BUFLEN 200
#define MIN_INTERVAL_S 800

/*=============================UI EVAS START HERE================================*/
/*Evas is visual stuff -

 * An Evas object is the most basic visual entity used in Evas.
 * Everything, be it a single line or a complex list of UI components, is an Evas object. */

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *navi;

} appdata_s;

Evas_Object *startHRM;
Evas_Object *event_label;
sensor_listener_h listener;

/* Registering a Callback for sensor event  */
void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data) {
	//TODO PASS THE TIME STUFF AS PARAMETER, COMP EXPENSIVE!
	time_t raw_time;
	struct tm* time_info;

	char out[100];

	// Select a specific sensor with a sensor handle
	sensor_type_e type;
	sensor_get_type(sensor, &type);

	switch (type) {

	case SENSOR_HRM:
		//print mesurement to console
		dlog_print(DLOG_DEBUG, LOG_TAG, "%f", event->values[0]);
		sprintf(out, "%f", event->values[0]);
		//elm_object_text_set(event_label, out);
		//insert timestamp
		time(&raw_time);
		time_info = localtime(&raw_time);
		dlog_print(DLOG_INFO, LOG_TAG,
				"Current time: %d:%s%d:%ds /Value HRM = %f", time_info->tm_hour,
				time_info->tm_min < 10 ? "0" : "", time_info->tm_min,
				time_info->tm_sec, event->values[0]);

		break;
	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "Not an HRM event");
	}
}


void _sensor_accuracy_changed_cb(sensor_h sensor, unsigned long long timestamp,
		sensor_data_accuracy_e accuracy, void *data) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor accuracy callback invoked");
	sensor_event_s event;

	int error = sensor_listener_read_data(listener, &event);
	dlog_print(DLOG_DEBUG, LOG_TAG, "accuracy is: %d", event.accuracy);
}

Evas_Object *new_button(appdata_s *ad, Evas_Object *parrent, char *name,
		void *action) {

	// Create a button
	Evas_Object *bt = elm_button_add(parrent);
	elm_object_text_set(bt, name);
	evas_object_smart_callback_add(bt, "clicked", (Evas_Smart_Cb) action, ad);
	evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(parrent, bt);
	evas_object_show(bt);
	return bt;
}

char* model_get_app_data_path() {
	char* path = NULL;

	char *path_tmp = app_get_shared_resource_path();
	if (!path_tmp) {
		dlog_print(DLOG_ERROR, LOG_TAG, "Function app_get_data_path() failed");
		return false;
	}

	*path = strdup(path_tmp);
	return path_tmp;
}

//static char* write_file(char *filepath, const char* buf) {
void write_file(const char* buf) {
	char *file = "datafile.txt";
	//malloc this shit

	//strcat(, file);
	//char* filepath ="/opt/usr/globalapps/org.example.hellotizen/shared/res/datafilehardcode.txt";
	//char filepath =  "datafile.txt";
	FILE *fp;
	fp =
			fopen(
					"/opt/usr/apps/org.example.hellotizen/shared/data/datafilehardcode.txt",
					"w");

	fputs(buf, fp);
	fclose(fp);
}

static char* read_file(const char* filepath) {
	FILE *fp = fopen(filepath, "r");
	if (fp == NULL)
		return NULL;
	fseek(fp, 0, SEEK_END);
	int bufsize = ftell(fp);
	rewind(fp);
	if (bufsize < 1)
		return NULL;
	char*buf = malloc(sizeof(char) * (bufsize));
	memset(buf, '\0', sizeof(buf));
	char str[200];
	while (fgets(str, 200, fp) != NULL) {
		dlog_print(DLOG_ERROR, "READFILE_CUSTOMTAG", "%s", str);
		sprintf(buf + strlen(buf), "%s", str);
	}
	fclose(fp);
	return buf;
}

void _sensor_stop_cb(void *data, Evas_Object *obj, void *event_info) {

}

void _sensor_start_cb(void *data, Evas_Object *obj, void *event_info) {

	time_t raw_time;
	struct tm* time_info;

	void *user_data = NULL;
	char out[100];
	int min_interval = 800;

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
	error = sensor_create_listener(sensor, &listener);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d",
				error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_create_listener");

	/* Register a callback to be invoked when sensor events are delivered via a sensor listener [above]  */
	error = sensor_listener_set_event_cb(listener, MIN_INTERVAL_S,
			on_sensor_event, user_data);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_set_event_cb error: %d", error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_event_cb");

	/* set the interval for HRM in milliseconds. 100-1000ms */
	error = sensor_listener_set_interval(listener, MIN_INTERVAL_S);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_set_interval error: %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_intervals");

	// Registering the Accuracy Changed Callback, not usre how it works as it does only take input. Perhaps sensor calibration?
	error = sensor_listener_set_accuracy_cb(listener,
			_sensor_accuracy_changed_cb, user_data);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG,
				"sensor_listener_set_accuracy_cb error: %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_accuracy_cb");

	/* Changes the power-saving behavior of a sensor listener. [ALWAYS ON] */
	error = sensor_listener_set_option(listener, SENSOR_OPTION_ALWAYS_ON);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_option");

	/* START the sensor listener */
	error = sensor_listener_start(listener);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_start");

	/* Read sensor data (from started listener). */
	sensor_event_s event;
	error = sensor_listener_read_data(listener, &event);
	if (error != SENSOR_ERROR_NONE) {

		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_read_data error: %d",
				error);
		return;
	}

	switch (type) {

	case SENSOR_HRM:
		//print mesurement to console
		dlog_print(DLOG_INFO, LOG_TAG, "%f", event.values[0]);
		sprintf(out, "%f", event.values[0]);

		//elm_object_text_set(event_label, out);
		//insert timestamp
		time(&raw_time);
		time_info = localtime(&raw_time);
		/*dlog_print(DLOG_INFO, LOG_TAG,
		 "Current time: %d:%s%d:%ds /Value HRM = %f", time_info->tm_hour,
		 time_info->tm_min < 10 ? "0" : "", time_info->tm_min, time_info->tm_sec, event.values[0]
		 );
		 */
		break;
	default:
		dlog_print(DLOG_ERROR, LOG_TAG, "Not an HRM event");
	}

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
	/* Create and initialize evas_conformant.
	 elm_conformant is mandatory for base gui to have proper size
	 when indicator or virtual keypad is visible. */

	/*Add a conformant widget to the given parent Elementary (container) object (win)*/
	ad->conform = elm_conformant_add(ad->win);


	/*[NOT A SIZE ENFORCEMENT] - hint on how a container object should resize a given child within
	 * its area, primitive EVAS_HINT_EXPAND */
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);

	/* adds subobj as a resize object of the window obj */
	elm_win_resize_object_add(ad->win, ad->conform);

	/* Make the conformant visible */
	evas_object_show(ad->conform);

	/* Make the window visible */
	evas_object_show(ad->win);

	// Create a naviframe
	ad->navi = elm_naviframe_add(ad->conform);
	evas_object_size_hint_align_set(ad->navi, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->navi, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);

	elm_object_content_set(ad->conform, ad->navi);

	//not sure? eext_object_event_callback_add(ad->navi, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);

	evas_object_show(ad->navi);

	//Lets call new button, testing - bug, replaces label!
	Evas_Object *box = elm_box_add(ad->conform);
	elm_object_content_set(ad->conform, box);
	startHRM = new_button(ad, box, "Start", _sensor_start_cb);

}
/*=============================UI EVAS END HERE================================*/

static bool app_create(void *data) {
	//similar to OnCreate() ???

	/* Hook to take necessary actions before main event loop starts
	 Initialize UI resources and application's data
	 If this function returns true, the main loop of application starts
	 If this function returns false, the application is terminated */
	//appdata_s *ad = data;
	//create_base_gui(ad);
	create_base_gui((appdata_s *) data);

	return true;
}

static void app_terminate(void *data) {
	/* Release all resources. */
}

int main(int argc, char *argv[]) {
	char* test = "Petar was here";
	char* path_to_;
	appdata_s ad;
	memset(&ad, 0x00, sizeof(appdata_s));
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;

	//char* default_path = app_get_shared_data_path();
	//path_to_ = model_get_app_data_path();
	//write_file(test);

	//char* file_write = write_file(path_to_,test);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
