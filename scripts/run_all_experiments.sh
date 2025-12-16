#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

echo "Run prio with win"
run_experiment "prio" "win"

echo "Run rr with win"
run_experiment "rr" "win"

echo "Run fq no win"
run_experiment "fq" "no_win"

echo "Run fq no win"
run_experiment "fq" "win"

echo "Run loom with win"
run_experiment "loom" "win"