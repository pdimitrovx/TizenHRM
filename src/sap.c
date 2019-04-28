#include "hellotizen.h"
#include <glib.h>
#include <sap.h>
#include <sap_message_exchange.h>
#include <time.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "SAP_debug"

#define MEX_PROFILE_ID "/sample/hellomessage"

struct priv {
	sap_agent_h agent;
	sap_peer_agent_h peer_agent;
};

gboolean is_agent_added = FALSE;
static struct priv priv_data = { 0 };

void waitFor(unsigned int secs) {
	unsigned int retTime = time(0) + secs;   // Get finishing time.
	while (time(0) < retTime)
		;               // Loop until it arrives.
}

void mex_message_delivery_status_cb(sap_peer_agent_h peer_agent_h,
		int transaction_id, sap_connectionless_transfer_status_e status,
		void *user_data);

void mex_message_delivery_status_cb(sap_peer_agent_h peer_agent_h,
		int transaction_id, sap_connectionless_transfer_status_e status,
		void *user_data) {
	dlog_print(DLOG_DEBUG, LOG_TAG,
			"sap_pa_message_delivery_status_cb:  transaction_id:%d, status:%d",
			transaction_id, status);
}

void mex_send(char *message, int length, gboolean is_secured) {
	int result;
	sap_peer_agent_h pa = priv_data.peer_agent;

	dlog_print(DLOG_DEBUG, LOG_TAG, "pa:%u, length :%d, message:%s", pa, length,
			message);

	if (sap_peer_agent_is_feature_enabled(pa, SAP_FEATURE_MESSAGE)) {
		result = sap_peer_agent_send_data(pa, (unsigned char *) message, length,
				is_secured, mex_message_delivery_status_cb, NULL);
		if (result <= 0) {
			dlog_print(DLOG_DEBUG, LOG_TAG, "Error in sending mex data");
			free(message);
			dlog_print(DLOG_DEBUG, LOG_TAG, "try again or check error val , %d",
					result);
		}
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG,
				"MEX is not supported by the Peer framework");
		update_ui("Message feature is not supported by the Peer");
		//Fallback to socket connection
	}

}

void mex_data_received_cb(sap_peer_agent_h peer_agent,
		unsigned int payload_length, void *buffer, void *user_data) {

	//chars to hold the extracted command parameters from the command message (received) - initialize
	char *received_sensor_type = g_strdup_printf("-1");
	char *received_sensor_time = g_strdup_printf("-1");

	unsigned int response_payload_length;
	//char *values = read_sensor_data();

	/*
	 * g_strdup_printf()
	 * Similar to the standard C sprintf() function but safer, since it calculates
	 * the maximum space required and allocates memory to hold the result.
	 * The returned string should be freed with g_free() when no longer needed.
	 */

	/* seperate the string using strtok() using delimiter, success */

	// copy buffer to work with
	char* str = g_strdup_printf("%s", (char *) buffer);
	if (strcmp(str, "STOP") != 0) {

		char* ptr;
		char* saved;
		int a = 0;

		ptr = strtok_r(str, ":", &saved);
		response_payload_length = strlen(ptr);
		mex_send(ptr, response_payload_length, FALSE); //command replaced msg2!!
		received_sensor_time = g_strdup_printf("%s", ptr);

		while (ptr != NULL) {
			for (a = 0; a < 2; a = a + 1) {
				ptr = strtok_r(NULL, ":", &saved);
				switch (a) {
				case 0:
					received_sensor_type = g_strdup_printf("%s", ptr);
					break;
				case 1:
					//more data for the future
					//received_sensor_type = g_strdup_printf("%s", ptr);
					break;
				default:
					break;
				}
			}

		}
		start_streaming_data(received_sensor_type);
		//test return the command data, split in strings, ok
		/*
		 msg3 = g_strdup_printf("Time: %s", (char *) received_sensor_time);
		 response_payload_length = strlen(msg3);
		 mex_send(msg3, response_payload_length, FALSE);
		 g_free(msg3);

		 msg4 = g_strdup_printf("Type: %s", (char *) received_sensor_type);
		 response_payload_length = strlen(msg4);
		 mex_send(msg4, response_payload_length, FALSE);
		 g_free(msg4);

		 */
		priv_data.peer_agent = peer_agent;

		//Free memory
		g_free(str);
		g_free(received_sensor_time);
		g_free(received_sensor_type);
	} else {
		//stop reading data?
		stop_sensor_ALL();
	}
}
void start_streaming_data(char *type) {
	//precautioin
	stop_sensor_ALL();

	char *hrm_type = "0";
	if (strcmp(type, hrm_type) == 0) {
		//start Heart Rate Sensor
		_sensor_start_cb_HRM();
	} else if ((strcmp(type, hrm_type) == 1)) {
		_sensor_start_cb_ACCELEROMETER();
	} else if ((strcmp(type, hrm_type) == 2)) {
		_sensor_start_cb_ACCELEROMETER();
		_sensor_start_cb_HRM();

	}}
	void on_peer_agent_updated(sap_peer_agent_h peer_agent,
			sap_peer_agent_status_e peer_status,
			sap_peer_agent_found_result_e result, void *user_data) {
		switch (result) {
		case SAP_PEER_AGENT_FOUND_RESULT_DEVICE_NOT_CONNECTED:
			dlog_print(DLOG_DEBUG, LOG_TAG, "device is not connected");
			break;

		case SAP_PEER_AGENT_FOUND_RESULT_FOUND:

			if (peer_status == SAP_PEER_AGENT_STATUS_AVAILABLE) {
				priv_data.peer_agent = peer_agent;
			} else {
				sap_peer_agent_destroy(peer_agent);
				priv_data.peer_agent = NULL;
			}
			break;

		case SAP_PEER_AGENT_FOUND_RESULT_SERVICE_NOT_FOUND:
			dlog_print(DLOG_DEBUG, LOG_TAG, "service not found");
			break;

		case SAP_PEER_AGENT_FOUND_RESULT_TIMEDOUT:
			dlog_print(DLOG_DEBUG, LOG_TAG, "peer agent find timed out");
			break;

		case SAP_PEER_AGENT_FOUND_RESULT_INTERNAL_ERROR:
			dlog_print(DLOG_DEBUG, LOG_TAG, "peer agent find search failed");
			break;
		}
	}

	static gboolean _find_peer_agent() {
		sap_result_e result = SAP_RESULT_FAILURE;

		result = sap_agent_find_peer_agent(priv_data.agent,
				on_peer_agent_updated,
				NULL);

		if (result == SAP_RESULT_SUCCESS) {
			dlog_print(DLOG_DEBUG, LOG_TAG, "find peer call succeeded");
		} else {
			dlog_print(DLOG_DEBUG, LOG_TAG,
					"findsap_peer_agent_s is failed (%d)", result);
		}
		dlog_print(DLOG_DEBUG, LOG_TAG, "find peer call is over");
		return FALSE;
	}

	static void on_agent_initialized(sap_agent_h agent,
			sap_agent_initialized_result_e result, void *user_data) {

		switch (result) {
		case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:
			dlog_print(DLOG_INFO, LOG_TAG, "agent is initialized");

			priv_data.agent = agent;
			sap_agent_set_data_received_cb(agent, mex_data_received_cb, NULL);
			is_agent_added = TRUE;

			_find_peer_agent();
			break;

		case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
			dlog_print(DLOG_DEBUG, LOG_TAG, "duplicate registration");
			break;

		case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
			dlog_print(DLOG_DEBUG, LOG_TAG, "invalid arguments");
			break;

		case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
			dlog_print(DLOG_DEBUG, LOG_TAG, "internal sap error");
			break;

		default:
			dlog_print(DLOG_DEBUG, LOG_TAG, "unknown status (%d)", result);
			break;
		}

		dlog_print(DLOG_DEBUG, LOG_TAG, "agent initialized callback is over");

	}

	static void on_device_status_changed(sap_device_status_e status,
			sap_transport_type_e transport_type, void *user_data) {
		switch (transport_type) {
		case SAP_TRANSPORT_TYPE_BT:
			dlog_print(DLOG_INFO, LOG_TAG, "connectivity type(%d): bt",
					transport_type);

			switch (status) {
			case SAP_DEVICE_STATUS_DETACHED:
				dlog_print(DLOG_DEBUG, LOG_TAG, "DEVICE GOT DISCONNECTED");
				sap_peer_agent_destroy(priv_data.peer_agent);
				priv_data.peer_agent = NULL;
				break;

			case SAP_DEVICE_STATUS_ATTACHED:
				if (is_agent_added == TRUE) {
					_find_peer_agent();
				}
				dlog_print(DLOG_DEBUG, LOG_TAG,
						"DEVICE IS CONNECTED NOW, PLEASE CALL FIND PEER");
				break;

			default:
				dlog_print(DLOG_DEBUG, LOG_TAG, "unknown status (%d)", status);
				break;
			}

			break;

		default:
			dlog_print(DLOG_DEBUG, LOG_TAG, "unknown connectivity type (%d)",
					transport_type);
			break;
		}
	}

	gboolean agent_initialize() {
		int result = 9;
		int attempts = 0;

		for (attempts = 0; attempts < 150; attempts++) {
			if (result != SAP_RESULT_SUCCESS) {
				waitFor(1);
				//((result != SAP_RESULT_SUCCESS))
				result = sap_agent_initialize(priv_data.agent, MEX_PROFILE_ID,
						SAP_AGENT_ROLE_PROVIDER, on_agent_initialized, NULL);
				dlog_print(DLOG_DEBUG, LOG_TAG,
						"SAP >>> sap_agent_initialize() >>> %d", result);
			}
		}

		if (result != SAP_RESULT_SUCCESS) {
			dlog_print(DLOG_DEBUG, LOG_TAG,
					"SAP >>> sap_agent_initialize() >>> ERROR/timeouts ");
			return FALSE;
		} else
			return TRUE;
	}

	void initialize_sap() {

		bool is_agent_initialized = FALSE;
		sap_agent_h agent = NULL;

		sap_agent_create(&agent);

		if (agent == NULL)
			dlog_print(DLOG_DEBUG, LOG_TAG, "ERROR in creating SAP agent");
		else {
			dlog_print(DLOG_DEBUG, LOG_TAG, "Successfully created SAP agent");
			dlog_print(DLOG_DEBUG, LOG_TAG,
					"Beginning initialization of SAP agent...");

			priv_data.agent = agent;

			sap_set_device_status_changed_cb(on_device_status_changed, NULL);

			is_agent_initialized = agent_initialize();
			if (is_agent_initialized) {
				dlog_print(DLOG_DEBUG, LOG_TAG,
						"Succesfully initialized SAP Agent!");
			}

		}

	}

