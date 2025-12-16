MAX_JOBS=6
PARENT_DIR=$(dirname "$0")

function run_experiment() {
  local name=$1
  local win=$2
  for i in {10..500}; do
    local wl_path="$BUFFY_WLS_DIR/$name/$name.${i}.txt"
    if [ -f "$wl_path" ]; then
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

function check_with_fperf() {
  local name=$1
  for i in {10..500}; do
    local wl_path="$BUFFY_WLS_DIR/$name/$name.${i}.txt"
    if [ -f "$wl_path" ]; then
      echo "Found workload file for buf_size=${i}"
      while [ "$(jobs -r | wc -l)" -ge "$MAX_JOBS" ]; do
         wait -n 2>/dev/null || wait
      done
      (
        export FPERF_WORKLOAD_PATH=/dev/null
        fperf-checker "$name" "$i" "$wl_path"
        echo "Run for buf_size=${i} completed!"
      ) &
    fi
  done
  wait
  echo "Workloads checked with fperf Successfully!"
}

function draw_experiment_chart(){
  local name=$1
  python3 "$PARENT_DIR/draw_chart.py" -l "$BUFFY_LOGS_DIR" -n "$name"
}
