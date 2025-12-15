#!/bin/bash

set -euo pipefail

BUILD_DIR="build/Release"
TARGET=$1

export BUFFY_WLS_DIR="data/wls"
export BUFFY_LOGS_DIR="data/logs"
MAX_JOBS=6

for i in {10..500}; do
  FILE_PATH="$BUFFY_WLS_DIR/$TARGET/$TARGET.${i}.txt"

  if [ -f "$FILE_PATH" ]; then
    while [ "$(jobs -r | wc -l)" -ge "$MAX_JOBS" ]; do
       wait -n 2>/dev/null || wait
    done
    (
      echo "${i}.txt exists"
      ./"$BUILD_DIR/$TARGET" "$i" true
    ) &
  fi
done

wait
echo "Experiment Completed Successfully!"