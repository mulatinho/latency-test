# Latency Test

A simple tool to measure network latency and response times.

## Features
- Measure latency to a given target (IP or hostname)
- Support for REDIS, PostgreSQL and Kafka
- Customizable interval and timeout settings
- Logging and reporting capabilities

## Installation

### Prerequisites

Ensure you have the following dependencies installed:

- curl 
- librdkafka-dev 
- libpq-dev libhiredis-dev 
- gcc 
- default-jre

### Compile the project

```sh
git clone https://github.com/mulatinho/latency-test.git
cd latency-test
make &&  make install
gcc -o latency-test latency-test.c -lrdkafka -lpq -lhiredis -I /usr/include/postgresql/ -I/usr/include/hiredis
```

## Usage

### Basic Example
```sh
./check-latency [postgresql|redis|kafka] <host> <port>

export PGUSER=youruser
export PGPASS=y0urpass
./check-latency postgresql 127.0.0.1 5432
```

### Example Output

```sh
$ PGUSER=yourus3r PGPASS=p4ssword ./check-latency postgresql 127.0.0.1 5432
:. [postgres->connection] milliseconds: 29.197785ms
:. [postgres->response] milliseconds: 20.253507ms
:. [postgres->total] milliseconds: 49.451292ms
:. [postgres->connection] milliseconds: 7.780328ms
:. [postgres->response] milliseconds: 0.797303ms
:. [postgres->total] milliseconds: 8.577631ms
:. [postgres->connection] milliseconds: 15.799209ms
:. [postgres->response] milliseconds: 1.347916ms
:. [postgres->total] milliseconds: 17.147125ms
:. [postgres->connection] milliseconds: 16.352700ms
:. [postgres->response] milliseconds: 1.524737ms
:. [postgres->total] milliseconds: 17.877437ms
:. [postgres->connection] milliseconds: 15.313719ms
:. [postgres->response] milliseconds: 1.438259ms
:. [postgres->total] milliseconds: 16.751978ms
:. [postgres->connection] milliseconds: 16.604589ms
:. [postgres->response] milliseconds: 1.794023ms
:. [postgres->total] milliseconds: 18.398612ms
:. [postgres->connection] milliseconds: 15.953160ms
:. [postgres->response] milliseconds: 1.437696ms
:. [postgres->total] milliseconds: 17.390856ms
:. [postgres->connection] milliseconds: 16.748397ms
:. [postgres->response] milliseconds: 1.436527ms
:. [postgres->total] milliseconds: 18.184924ms
:. [postgres->connection] milliseconds: 16.156682ms
:. [postgres->response] milliseconds: 1.534100ms
:. [postgres->total] milliseconds: 17.690782ms
:. [postgres->connection] milliseconds: 16.197249ms
:. [postgres->response] milliseconds: 1.501010ms
:. [postgres->total] milliseconds: 17.698259ms
:. [postgresql] latency in average is: 19.916890ms (10 tests were executed)
```

### Kubernetes Support

Deploying in Kubernetes can be done by using the image below and the Kubernetes manifest
```sh
docker pull mulatinho/latency-test:latest
kubectl apply -f extra/latency-deployment.yaml
```

### Exposing as Prometheus Metrics
SOON.

## Contributing
Contributions are welcome! Please follow these steps:
1. Fork the repository
2. Create a new branch (`git checkout -b feature-branch`)
3. Commit your changes (`git commit -m 'Add new feature'`)
4. Push to your fork (`git push origin feature-branch`)
5. Open a pull request

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact
For questions or suggestions, feel free to reach out via GitHub issues.

