#include "mlt.h"
#include "../latency-test.h"

void unit_test_get_ip(void)
{
	char *result = get_ip("localhost");
	fprintf(stdout, "result: %s\n", result);
	mlt_streq(result, "127.0.0.1");
}

int main(void)
{
    mlt_start();

	unit_test_get_ip();

    mlt_finish();
}
