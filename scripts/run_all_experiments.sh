#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

echo "Run prio with win"
run_experiment "prio" 1

echo "Run rr with win"
run_experiment "rr" 1

#echo "Run fq no win"
#run_experiment "fq" 0
#
#echo "Run fq no win"
#run_experiment "fq" 1
#
#echo "Run loom with win"
#run_experiment "loom" 1
