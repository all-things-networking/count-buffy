MAX_JOBS=6
PARENT_DIR=$(dirname "$0")

cleanup() {
    echo -e "Killing all running jobs..."
    kill 0
    exit 1
}

trap cleanup SIGINT

function run_experiment() {
  local name=$1
  local win=$2
  for i in $(seq "$BUFFY_MIN_BUF_SIZE" "$BUFFY_MAX_BUF_SIZE"); do
    local wl_path="$BUFFY_WLS_DIR/$name/$name.${i}.txt"
    if [ -f "$wl_path" ]; then
      echo "Found workload file for buf_size=${i}"
      while [ "$(jobs -r | wc -l)" -ge "$MAX_JOBS" ]; do
         wait -n 2>/dev/null || wait
      done
      (
        $name "$i" "$win"
        echo "Buffy run for buf_size=${i} completed!"
      ) &
    fi
  done
  wait
  echo "Experiment Completed Successfully!"
}

function run_fperf_search() {
  local name=$1
  for i in $(seq "$BUFFY_MIN_BUF_SIZE" "$BUFFY_MAX_BUF_SIZE"); do
    local sub_wl_path="data/sub_wls/$name/$name.${i}.txt"
    if [ -f "$sub_wl_path" ]; then
      echo "Found workload file for buf_size=${i}"
      while [ "$(jobs -r | wc -l)" -ge "$MAX_JOBS" ]; do
         wait -n 2>/dev/null || wait
      done
      mkdir -p "data/new_wls/$name"
      (
        export FPERF_OUTPUT_WL_PATH="data/new_wls/$name/$name.${i}.txt"
        fperf-search "$name" "$i"
        echo "Fperf search for buf_size=${i} completed!"
      ) &
    fi
  done
  wait
  echo "Fperf search completed Successfully!"
}

function check_with_fperf() {
  local name=$1
  for i in $(seq "$BUFFY_MIN_BUF_SIZE" "$BUFFY_MAX_BUF_SIZE"); do
    local wl_path="$BUFFY_WLS_DIR/$name/$name.${i}.txt"
    if [ -f "$wl_path" ]; then
      echo "Found workload file for buf_size=${i}"
      while [ "$(jobs -r | wc -l)" -ge "$MAX_JOBS" ]; do
         wait -n 2>/dev/null || wait
      done
      (
        export FPERF_OUTPUT_WL_PATH=/dev/null
        fperf-checker "$name" "$i" "$wl_path"
        echo "Fperf check for buf_size=${i} completed!"
      ) &
    fi
  done
  wait
  echo "Workloads checked with fperf Successfully!"
}

function draw_experiment_chart(){
  local name=$1
  python3 "$PARENT_DIR/draw_chart.py" -w "$BUFFY_WLS_DIR" -l "$BUFFY_LOGS_DIR" \
  -n "$name" \
  --min "$BUFFY_MIN_BUF_SIZE" \
  --max "$BUFFY_MAX_BUF_SIZE"
}
