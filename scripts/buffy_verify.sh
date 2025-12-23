#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

run_experiment "$1"
