#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

echo "Checking prio workloads with fperf"
check_with_fperf "prio"

echo "Checking rr workloads with fperf"
check_with_fperf "rr"

echo "Checking fq workloads with fperf"
check_with_fperf "fq"

echo "Checking loom workloads with fperf"
check_with_fperf "loom"

