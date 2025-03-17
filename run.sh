set -euo pipefail

make -j6
LOG_FILE=$1
echo > $LOG_FILE
for ((i=5; i<=500; i = i + 5)); do
  gtime -f "$i,%e" -o $LOG_FILE -a ./a.out 10 $i 10
#  (time ./build/fperf 10) 2>&1 | grep cpu | awk '{print $9}'
#  echo "$i done"
done