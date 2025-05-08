FROM debian:bookworm
ENV PATH=$PATH:/opt/app/kafka_2.12-2.0.1/bin

WORKDIR /app

RUN apt-get update -y && \
	apt-get install build-essential make curl librdkafka-dev libpq-dev libhiredis-dev gcc default-jre -y

COPY . /app/

RUN make

FROM debian:bookworm-slim 
WORKDIR /app
COPY --from=0 /app/src/check-latency /app/
COPY --from=0 /app/src/latency-test.sh /app/

CMD [ "/app/latency-test.sh" ]
