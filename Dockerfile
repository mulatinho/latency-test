FROM debian:buster

WORKDIR /opt/app

RUN apt-get update -y && \
	apt-get install curl librdkafka-dev libpq-dev libhiredis-dev gcc default-jre -y

RUN curl -O http://mirror.nbtelecom.com.br/apache/kafka/2.5.0/kafka_2.12-2.5.0.tgz && \
	tar xfz kafka_2.12-2.5.0.tgz -C /opt/app/

ENV PATH=$PATH:/opt/app/kafka_2.12-2.5.0/bin

COPY latency-test.* /opt/app/

RUN gcc -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis

CMD [ "./latency-test.sh" ]
