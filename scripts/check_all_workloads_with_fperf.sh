#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

echo "Checking prio workloads with fperf"
check_with_fperf "prio" 1

echo "Checking rr workloads with fperf"
check_with_fperf "rr" 1

