# Count Buffy

## Getting Started

### Requirements

Install [Docker](https://docs.docker.com/get-started/get-docker/)

### Pull the Buffy Image

```shell
docker compose pull
```

### Hello World Example

The `examples` directory includes a simple example of using count-buffy to
model a rate-limiter scheduler and verify certain workload and queries.

```shell
docker compose run --rm buffy hello-world
```

### Run Experiments

Clear existing log files:

```shell
rm -rf data/logs
```

Run all experiments (rr and prio):

```shell
docker compose run --rm buffy run_all_experiments.sh
```

Logs files are created in the `data/logs` directory.

Draw all charts (rr and prio):

```shell
docker compose run --rm buffy draw_all_charts.sh
```

Charts are saved in the `data/charts` directory.

### Check workloads with FPerf
Check workloads with FPerf:

```shell
docker compose run --rm buffy check_all_workloads_with_fperf.sh
```


