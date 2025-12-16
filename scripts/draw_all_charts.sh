#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

for name in "prio" "rr" "fq" "loom"; do
  draw_experiment_chart "$name"
done


