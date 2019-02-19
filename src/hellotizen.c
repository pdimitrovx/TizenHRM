//#include "hellotizen.h"
#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "hellotizen"

#if !defined(PACKAGE)
#define PACKAGE "org.example.hellotizen"
#endif


/*=============================UI EVAS START HERE================================*/
/*Evas is visual stuff -

 * An Evas object is the most basic visual entity used in Evas.
 * Everything, be it a single line or a complex list of UI components, is an Evas object. */

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
} appdata_s;

Evas_Object *new_button(Evas_Object *parrent, char *name, void *action){

	// Create a button
	    Evas_Object *bt = elm_button_add(parrent);
	    elm_object_text_set(bt, name);
	    evas_object_smart_callback_add(bt, "clicked", (Evas_Smart_Cb) action, ad);
	    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
	    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
	    elm_box_pack_end(display, bt);
	    evas_object_show(bt);
	    return bt;

}
Evas_object *parrent;


button = elm_button_add(parrent);
elm_object_text_set(new_button,"Read HRS");
elm_object_style_set(new_button, "bottom");


static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Label */
	/* Create an actual view of the base gui.
	   Modify this part to change the view. */
	ad->label = elm_label_add(ad->conform);
	elm_object_text_set(ad->label, "<align=center>Hello Tizen</align>");
	evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->label);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}
/*=============================UI EVAS END HERE================================*/

static bool
app_create(void *data)
{
	//similar to OnCreate() ???

	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	return true;
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}


int
main(int argc, char *argv[])
{
	appdata_s ad;
	    memset(&ad, 0x00, sizeof(appdata_s));	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;



	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
