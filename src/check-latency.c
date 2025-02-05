#include "latency-test.h"

int main(int argc, char **argv)
{
	int port = 0;

	if (argc < 3) { warn_error_and_quit(); }

	port = atoi(argv[3]);

	handle_measure(argv[1], argv[2], port);

	return 0;
}
