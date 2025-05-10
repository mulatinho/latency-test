/*
 * Latency tests on Kafka, Redis, PostgreSQL
 * Alexandre Mulatinho - 2020
 *
 * requirements:
 * $ apt-get install postgresql-dev librdkafka-dev libpq-dev libhiredis-dev
 *
 * compile:
 * $ gcc -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis
 *
 * usage:
 * $ export POSTGRES_USER=postgres POSTGRES_PASSWORD=password KAFKATOPIC=latency KAFKAGROUP=latency\n");
 * $ ./latency-test postgresql 127.0.0.1 5432
 * $ ./latency-test kafka 127.0.0.1 9092
 * $ ./latency-test redis 127.0.0.1 6379
 *
 */

#include "latency-test.h"

void latency_warn_error_and_quit(void)
{
	fprintf(stderr, "usage: ./latency-test <kafka|redis|postgresql> 127.0.0.1 31200\n");
	exit(1);
}

double latency_calc(char *name, struct timespec start, struct timespec finish)
{
	long seconds = finish.tv_sec - start.tv_sec;
	long ns = finish.tv_nsec - start.tv_nsec;
	double result;

	if (start.tv_nsec > finish.tv_nsec)
	{
		--seconds;
		ns += 1000000000;
	}
	result = (double)ns / (double)1000000;
	fprintf(stdout, ":. [%s] milliseconds: %fms\n", name, result);

	return result;
}

char *latency_host_to_ip(char *hostname)
{
	struct hostent *host;
	struct sockaddr_in sock_addr;

	if ((host = gethostbyname(hostname)))
	{
		sock_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
		return inet_ntoa(sock_addr.sin_addr);
	}

	return NULL;
}

double latency_collect_postgresql(char *server_host, int port)
{
	char conninfo[NAME_MAX];
	PGconn *conn;
	struct timespec t1, t2, t3;
	double result = 0.0;

	char *pg_user = getenv("POSTGRES_USER");
	char *pg_pass = getenv("POSTGRES_PASSWORD");

	if (!pg_user || !pg_pass)
		snprintf(conninfo, sizeof(conninfo) - 1, "host=%s port=%d dbname=postgres user=postgres", server_host, port);
	else
		snprintf(conninfo, sizeof(conninfo) - 1, "host=%s port=%d dbname=postgres user=%s password=%s",
				 server_host, port, pg_user, pg_pass);

	clock_gettime(CLOCK_REALTIME, &t1);

	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK)
	{
		fprintf(stderr, ":. connection string: %s\n:. connection refused while trying to connect into %s...", conninfo, server_host);
		PQfinish(conn);
		exit(-1);
	}

	clock_gettime(CLOCK_REALTIME, &t2);
	latency_calc("postgres->connection", t1, t2);

	PQexec(conn, "select count(*) from pg_catalog.pg_type");

	clock_gettime(CLOCK_REALTIME, &t3);
	latency_calc("postgres->response", t2, t3);

	result = latency_calc("postgres->total", t1, t3);
	PQfinish(conn);

	return result;
}

double latency_collect_redis(char *server_host, int port)
{
	redisContext *redis_ctx;
	redisReply *reply;
	struct timespec t1, t2, t3;
	double result = 0.0;

	clock_gettime(CLOCK_REALTIME, &t1);

	redis_ctx = redisConnect(server_host, port);

	if (redis_ctx != NULL && redis_ctx->err)
	{
		fprintf(stderr, ":. connection refused while trying to connect into %s...", server_host);
		exit(-1);
	}

	clock_gettime(CLOCK_REALTIME, &t2);
	latency_calc("redis->connection", t1, t2);

	reply = redisCommand(redis_ctx, "KEYS *");
	clock_gettime(CLOCK_REALTIME, &t3);
	latency_calc("redis->response", t2, t3);
	freeReplyObject(reply);

	result = latency_calc("redis->total", t1, t3);
	redisFree(redis_ctx);

	return result;
}

double latency_collect_kafka(char *server_host, int port)
{
	rd_kafka_t *rk;
	rd_kafka_conf_t *conf;
	rd_kafka_topic_partition_list_t *subscription;
	struct timespec t1, t2, t3;
	double result = 0;
	char broker[NAME_MAX], errstr[NAME_MAX];
	char *topic_name;
	;
	int running = 1;

	clock_gettime(CLOCK_REALTIME, &t1);

	conf = rd_kafka_conf_new();

	if (rd_kafka_conf_set(conf, "client.id", server_host,
						  errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
	{
		goto config_failed;
	}

	snprintf(broker, sizeof(broker) - 1, "%s:%d", server_host, port);
	if (rd_kafka_conf_set(conf, "bootstrap.servers", broker,
						  errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
	{
		goto config_failed;
	}

	if (rd_kafka_conf_set(conf, "group.id", "latency",
						  errstr, sizeof(errstr)) != RD_KAFKA_CONF_OK)
	{
		goto config_failed;
	}

	if (!(rk = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr))))
	{
		fprintf(stderr, ":. Failed to create new consumer: %s\n", errstr);
		goto config_failed;
	}

	conf = NULL;

	rd_kafka_poll_set_consumer(rk);

	subscription = rd_kafka_topic_partition_list_new(1);

	if (!(topic_name = getenv("KAFKATOPIC")))
	{
		fprintf(stderr, ":. You need to set KAFKATOPIC environment variable\n");
		goto config_failed;
	}

	rd_kafka_topic_partition_list_add(subscription, topic_name, RD_KAFKA_PARTITION_UA);

	if (rd_kafka_subscribe(rk, subscription) != RD_KAFKA_RESP_ERR_NO_ERROR)
	{
		fprintf(stderr, ":. failed to subscribe on topic\n");
		goto config_failed;
	}

	rd_kafka_topic_partition_list_destroy(subscription);

	clock_gettime(CLOCK_REALTIME, &t2);
	latency_calc("kafka->connection", t1, t2);

	while (running)
	{
		rd_kafka_message_t *rkmessage = rd_kafka_consumer_poll(rk, 100);
		if (rkmessage)
		{
			LATENCY_DEBUG(":. Message fetched on topic %s, metadata:(offset %ld, %zd bytes):\n",
					rd_kafka_topic_name(rkmessage->rkt), rkmessage->offset, rkmessage->len);

			clock_gettime(CLOCK_REALTIME, &t3);
			latency_calc("kafka->fetch_topic", t1, t3);

			rd_kafka_message_destroy(rkmessage);
			running--;
		}
	}

	result = latency_calc("kafka->total", t1, t3);
	goto exit_nicely;

config_failed:
	fprintf(stderr, ":. critical error: %s\n", errstr);
	rd_kafka_destroy(rk);

	exit(2);

exit_nicely:
	rd_kafka_destroy(rk);
	return result;
}

int latency_collect_metric(char *type, char *hostname, int port, double latency)
{
	return 0;
}

int latency_update_metrics(void)
{
	char *svc_enabled[] = { "REDISENABLE", "KAFKAENABLE", "POSTGRESENABLE" };
	char *env_list[][] = {
		{ "REDIS_HOST", "REDIS_PORT" },
		{ "KAFKA_HOST", "KAFKA_PORT", "ZOOKEEPER_HOST", "KAFKA_TOPIC" },
		{ "POSTGRES_HOST", "POSTGRES_PORT", "POSTGRES_USER", "POSTGRES_PASSWORD" },
	};
	char buffer[BUFFER_SIZE] = {0};
	int svc_enabled_size = sizeof(svc_enabled) / sizeof(svc_enabled[0]);

	snprintf(buffer, sizeof(buffer) - 1,
		"# HELP latency_test_total_measures Total of connections initiated from latency test\n"
		"# TYPE latency_test_total_measures counter\n");

	for (int svc_loop = 0; svc_loop < svc_enabled_size; svc_loop++) {
		char *env = getenv(svc_enabled[svc_loop]);
		char *host = NULL, *port = NULL;

		if (!strncmp(env, "true", 4)) {
			double miliseconds = 0;
			env_list_size = sizeof(env_list[svc_loop]) / sizeof(env_list[svc_loop][0]);

			switch(svc_loop) {
			case LATENCY_SERVICE_REDIS:
				miliseconds = latency_collect("redis", host, port);

				snprintf(buffer, sizeof(buffer) - 1,
				 	"%s\nlatency_test_measure_miliseconds{service=\"%s\",hostname=\"%s\",port=\"%d\"} %f\n",
				 	buffer, "redis", host, port, miliseconds);
				break;
			case LATENCY_SERVICE_KAFKA:
				latency_collect("kafka", host, port);
				snprintf(buffer, sizeof(buffer) - 1,
				 	"%s\nlatency_test_measure_miliseconds{service=\"%s\",hostname=\"%s\",port=\"%d\"} %f\n",
				 	buffer, "kafka", host, port, miliseconds);
				break;
			case LATENCY_SERVICE_POSTGRES:
				latency_collect("postgres", host, port);
				snprintf(buffer, sizeof(buffer) - 1,
				 	"%s\nlatency_test_measure_miliseconds{service=\"%s\",hostname=\"%s\",port=\"%d\"} %f\n",
				 	buffer, "postgres", host, port, miliseconds);
				break;
			}
		}
		// snprintf(buffer, sizeof(buffer) - 1,
		// 	"%s\nlatency_test_total_measures{container=\"latency-test\",service=\"%s\",hostname=\"%s\",port=\"%d\"} %f\n",
		// 	buffer, type, hostname, port, latency);
	}

	snprintf(buffer, sizeof(buffer) - 1,
		"%s\n# HELP latency_test_measure_miliseconds The latency average in miliseconds\n"
		"# TYPE latency_test_measure_miliseconds gauge\n", buffer);

	for (int svc_loop = 0; svc_loop < svc_enabled_size; svc_loop++) {
		// snprintf(buffer, sizeof(buffer) - 1,
		// 	"%s\nlatency_test_measure_miliseconds{container=\"latency-test\",service=\"%s\",hostname=\"%s\",port=\"%d\"} %f\n",
		// 	type, hostname, port, latency);
	}

	return 0;
}

int latency_collect(char *type, char *hostname, int port)
{
	int i = 0;
	double sum = 0.0, result = 0.0;
	double (*func)(char *, int);
	char *ipaddr = NULL;

	if (!strncmp(type, "kafka", 5))
		func = latency_collect_kafka;
	else if (!strncmp(type, "redis", 5))
		func = latency_collect_redis;
	else if (!strncmp(type, "postgresql", 10))
		func = latency_collect_postgresql;
	else latency_warn_error_and_quit();

	if ((ipaddr = latency_host_to_ip(hostname)) == NULL)
	{
		fprintf(stderr, ":. could not resolve the ip address: %s\n", hostname);
		exit(99);
	}

	while (i++ < QUANTITY)
	{
		sum += func(ipaddr, port);
		usleep(1500);
	}

	result = sum / QUANTITY;
	fprintf(stdout, ":. [%s] latency in average is: %fms (%d tests were executed)\n", type, result, QUANTITY);

	return 0;
}

int latency_start_server(void)
{
	int server_fd, new_connection;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	char buffer[BUFFER_SIZE] = {0};

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(LATENCY_PORT);
	bind(server_fd, (struct sockaddr *)&address, sizeof(address));

	listen(server_fd, 3);

	printf(":. latency-test: serving metrics on port %d\n", LATENCY_PORT);

	while (1)
	{
		new_connection = accept(server_fd, (struct sockaddr *)&address, &addrlen);
		if (new_connection < 0)
		{
			perror("accept");
			continue;
		}

		// TODO GET CONTENT

		// snprintf(buffer, sizeof(buffer),
		// 			"HTTP/1.1 200 OK\r\n"
		// 			"Content-Type: text/plain\r\n"
		// 			"Content-Length: %lu\r\n\r\n%s",
		// 			strlen(content), content);

		send(new_connection, buffer, strlen(buffer), 0);
		close(new_connection);

		usleep(850);
	}

	return 0;
}

int main(int argc, char **argv)
{
	int port = 0, opt = 0;

	if (argc < 3)
		latency_warn_error_and_quit();

	opt = getopt(argc, argv, "ghnuv");
	switch (opt)
	{
	case 'h':
		latency_warn_error_and_quit();
		break;
	case 'v':
		fprintf(stdout, LATENCY_VERSION);
		break;
	case 's':
		latency_start_server();
		break;
	default:
		break;
	}

	port = atoi(argv[3]);
	latency_collect(argv[1], argv[2], port);

	return 0;
}
