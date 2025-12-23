#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

all_experiments="$BUFFY_ALL_EXPERIMENTS"

for experiment in $all_experiments; do
    echo "Run fperf search for $experiment"
    run_fperf_search "$experiment"
done

