#!/bin/bash

set -euo pipefail

source "$(dirname "$0")/lib.sh"

check_with_fperf "$1"
