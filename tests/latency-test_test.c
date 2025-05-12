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

void unit_test_get_ip(void)
{
	int BATTERY_INPUT = 0, BATTERY_OUTPUT = 1;
	char *battery_test[][NAME_MAX] = {
		{ "localhost", "127.0.0.1" },
		{ "127.0.0.1", "127.0.0.1" },
		{ "dns.google", "8.8.8.8" },
	};
	int battery_size = sizeof(battery_test) / sizeof(battery_test[0]);

	for (int battery = 0; battery < battery_size; battery++) {
		char *result = latency_host_to_ip(battery_test[battery][BATTERY_INPUT]);
		mlt_streq(result, battery_test[battery][BATTERY_OUTPUT]);
	}
}

int main(void)
{
    mlt_start();

	unit_test_get_ip();

    mlt_finish();
}
