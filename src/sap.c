#include "hellotizen.h"
#include <glib.h>
#include <sap.h>
#include <sap_message_exchange.h>
#include <time.h>

#define MEX_PROFILE_ID "/sample/hellomessage"

struct priv {
	sap_agent_h agent;
	sap_peer_agent_h peer_agent;
};

gboolean is_agent_added = FALSE;
static struct priv priv_data = { 0 };

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
	char *msg = g_strdup_printf("Received data: %s", (char *) buffer);
	dlog_print(DLOG_INFO, LOG_TAG, "message:%s, length:%d", buffer,
			payload_length);

	priv_data.peer_agent = peer_agent;

	mex_send(buffer, payload_length, FALSE);
	int values = read_sensor_data();

	g_free(msg);
}

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

	result = sap_agent_find_peer_agent(priv_data.agent, on_peer_agent_updated,
	NULL);

	if (result == SAP_RESULT_SUCCESS) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "find peer call succeeded");
	} else {
		dlog_print(DLOG_DEBUG, LOG_TAG, "findsap_peer_agent_s is failed (%d)",
				result);
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

void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}

gboolean agent_initialize() {
	int result = 0;

	do {
		result = sap_agent_initialize(priv_data.agent, MEX_PROFILE_ID,
				SAP_AGENT_ROLE_PROVIDER, on_agent_initialized, NULL);
		dlog_print(DLOG_DEBUG, LOG_TAG,
				"SAP >>> getRegisteredServiceAgent() >>> %d", result);
		waitFor(1); //added delay if it fails. Maybe add a number of attempts?
	} while (result != SAP_RESULT_SUCCESS);

	return TRUE;
}

void initialize_sap() {
	sap_agent_h agent = NULL;
int error = 0;
	error = sap_agent_create(&agent);
	if (error == 0) {
		dlog_print(DLOG_DEBUG, "debugsap", "SAP >> No response");
	} else {
		switch (error) {

		case SAP_RESULT_PERMISSION_DENIED:
			dlog_print(DLOG_DEBUG, "debugsap" ,
					"SAP >> Create agent permission denied");
			break;
		case SAP_RESULT_FAILURE:
			dlog_print(DLOG_DEBUG, "debugsap",
					"SAP >> Create agent Result Failure");
			break;
		default:
			dlog_print(DLOG_DEBUG, "debugsap", "SAP >> Unknown error");
		}
	}

	if (agent == NULL)
		dlog_print(DLOG_DEBUG, LOG_TAG, "ERROR in creating agent");
	else
		dlog_print(DLOG_DEBUG, LOG_TAG, "successfully created sap agent");

	priv_data.agent = agent;

	sap_set_device_status_changed_cb(on_device_status_changed, NULL);

	agent_initialize();
}
