#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

all_experiments="$BUFFY_ALL_EXPERIMENTS"

for experiment in $all_experiments; do
  run_experiment "$experiment" "win"
  if [[ "$experiment" == "fq" ]]; then
    echo "Run fq no win"
    run_experiment "fq" "no_win"
  fi
done

