#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

all_experiments="$BUFFY_ALL_EXPERIMENTS"

for experiment in $all_experiments; do
  draw_plot "$experiment"
done


