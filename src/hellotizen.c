//#include "hellotizen.h"
#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <sensor.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "hellotizen"

#if !defined(PACKAGE)
#define PACKAGE "org.example.hellotizen"
#endif
#define BUFLEN 200

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
//Evas_object *parrent;
//button = elm_button_add(parrent);
//elm_object_text_set(new_button,"Read HRS");
//elm_object_style_set(new_button, "bottom");

bool model_get_app_data_path(char **path) {
	*path = NULL;

	char *path_tmp = app_get_shared_resource_path();
	if (!path_tmp) {
		dlog_print(DLOG_ERROR, LOG_TAG, "Function app_get_data_path() failed");
		return false;
	}

	*path = strdup(path_tmp);
	return true;
}

static char* write_file(char *filepath, const char* buf) {
	char *file = "datafile.txt";
	//malloc this shit

	//strcat(, file);
	filepath =
			"/opt/usr/globalapps/org.example.hellotizen/shared/res/datafilehardcode.txt";
	FILE *fp;
	fp = fopen(filepath, "w");
	fputs(buf, fp);
	fclose(fp);
	return NULL;
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
void _sensor_start_cb(void *data, Evas_Object *obj, void *event_info) {

	void *user_data = NULL;
	char out[100];

	// Retrieving a Sensor
	//check if sensor HRM is supported yo
	sensor_type_e type = SENSOR_HRM;
	sensor_h sensor;

	bool supported;
	int error = sensor_is_supported(type, &supported);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error);
		return;
	}

	if (supported) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "HRM is%s supported",
				supported ? "" : " not");
		sprintf(out, "HRM is%s supported", supported ? "" : " not");
		elm_object_text_set(event_label, out);
	}

	// Get sensor list
	//get handle sensor_h for the sensor hrm
	int count = 0;
	sensor_h *list;

	error = sensor_get_sensor_list(type, &list, &count);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_sensor_list error: %d",
				error);
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG, "Number of sensors: %d", count);
		free(list);
	}

	error = sensor_get_default_sensor(type, &sensor);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_get_default_sensor");
	// Registering a Sensor Event
	error = sensor_create_listener(sensor, &listener);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_create_listener");

	int min_interval = 0;
	error = sensor_get_min_interval(sensor, &min_interval);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_interval error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum interval of the sensor: %d",
			min_interval);

	float min_range = 0.0;
	error = sensor_get_min_range(sensor, &min_range);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_range error: %d",
				error);
		return;

	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum range of the sensor: %d",
			min_range);

	float max_range = 0.0;
	error = sensor_get_max_range(sensor, &max_range);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_max_range error: %d",
				error);
		return;
	}

	dlog_print(DLOG_DEBUG, LOG_TAG, "Maximum range of the sensor: %d",
			min_range);

	/** Get sensor details
	 @ToDo

	 char *name;
	 char *vendor;
	 sensor_type_e type;
	 float min_range;
	 float max_range;
	 float resolution;
	 int min_interval;

	 error = sensor_get_name(listener, &name);
	 error = sensor_get_vendor(listener, &vendor);
	 error = sensor_get_type(listener, &type);
	 error = sensor_get_min_range(listener, &min_range);
	 error = sensor_get_max_range(listener, &max_range);
	 error = sensor_get_resolution(listener, &resolution);
	 error = sensor_get_min_interval(listener, &min_interval);
	 */

}
static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info) {
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void create_base_gui(appdata_s *ad) {
	/* Window */
	/* Create and initialize elm_win.
	 elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win,
				(const int *) (&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request",
			win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb,
			ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	 elm_conformant is mandatory for base gui to have proper size
	 when indicator or virtual keypad is visible. */

	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Label
	 * Create an actual view of the base gui.
	 Modify this part to change the view.
	 ad->label = elm_label_add(ad->conform);
	 elm_object_text_set(ad->label, "<align=center>Hello Tizen</align>");
	 evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	 elm_object_content_set(ad->conform, ad->label);
	 */
	/* Show window after base gui is set up */
	evas_object_show(ad->win);

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

	//char * default_path = app_get_shared_data_path();
	//write_file(default_path,test);
	model_get_app_data_path(&path_to_);

	//char* file_write = write_file(path_to_,test);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
