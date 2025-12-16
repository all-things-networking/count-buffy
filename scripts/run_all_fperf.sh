#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

echo "Run fperf search for prio"
run_fperf_search "prio"

echo "Run fperf search for rr"
run_fperf_search "rr"

echo "Run fperf search for fq"
run_fperf_search "fq"

echo "Run fperf search for loom"
run_fperf_search "loom"
