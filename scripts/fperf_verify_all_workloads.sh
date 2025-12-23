#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

all_experiments="$BUFFY_ALL_EXPERIMENTS"

for experiment in $all_experiments; do
    echo "Checking $experiment workloads with fperf"
    check_with_fperf "$experiment"
done

