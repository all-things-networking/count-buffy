MAX_JOBS=6
PARENT_DIR=$(dirname "$0")

function run_experiment() {
  local name=$1
  local win=$2
  for i in {10..500}; do
    local WL_PATH="$BUFFY_WLS_DIR/$name/$name.${i}.txt"
    if [ -f "$WL_PATH" ]; then
      echo "Found workload file for buf_size=${i}"
      while [ "$(jobs -r | wc -l)" -ge "$MAX_JOBS" ]; do
         wait -n 2>/dev/null || wait
      done
      (
        $name "$i" "$win"
        echo "Run for buf_size=${i} completed!"
      ) &
    fi
  done
  wait
  echo "Experiment Completed Successfully!"
}

function draw_experiment_chart(){
  local name=$1
  python3 "$PARENT_DIR/draw_chart.py" -l "$BUFFY_LOGS_DIR" -n "$name"
}
