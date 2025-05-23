#include "mlt.h"
#include "../src/latency-test.h"

//gchar *latency_host_to_ip(char *hostname);
//gdouble latency_calc(char *name, struct timespec start, struct timespec finish);
//gdouble latency_collect_kafka(char *server_host, int port);
//gdouble latency_collect_postgresql(char *server_host, int port);
//gdouble latency_collect_redis(char *server_host, int port);
//gint latency_collect_metric(char *type, char *hostname, int port, double latency);
//gdouble latency_collect(char *type, char *hostname, int port);
//gint latency_update_metrics(char *);
//gint latency_start_server(void);
//gvoid latency_warn_error_and_quit(void);


#define MLT_TESTING 1

void unit_test_get_ip(void)
{
	int BATTERY_INPUT = 0, BATTERY_OUTPUT = 1;
	char *battery_test[][NAME_MAX] = {
		{ "localhost", "127.0.0.1" },
		{ "127.0.0.1", "127.0.0.1" },
		{ "lwn.net", "173.255.236.65" },
	};
	int battery_size = sizeof(battery_test) / sizeof(battery_test[0]);

	for (int battery = 0; battery < battery_size; battery++) {
		char *result = latency_host_to_ip(battery_test[battery][BATTERY_INPUT]);
		mlt_assert(result != NULL);
		mlt_streq(result, battery_test[battery][BATTERY_OUTPUT]);
	}

	mlt_assert(latency_host_to_ip("sdadi.dwsf") == NULL);
}

int main(void)
{
	mlt_start();
	mlt_suite_begin("unittests get_ip");

	unit_test_get_ip();
    	
	mlt_suite_end();

	mlt_finish();
}
