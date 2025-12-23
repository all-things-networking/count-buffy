#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

run_fperf_search "$1"
