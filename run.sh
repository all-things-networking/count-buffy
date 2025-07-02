set -euo pipefail

BUILD_DIR="cmake-build-debug"

cd $BUILD_DIR
make -j6
for ((j=100; j<=500; j = j + 100)); do
  for ((i=10; i<=150; i = i + 5)); do
    gtime -f "$i,%e" -o "loom.$j.txt" -a ./Loom $i $j
  #  (time ./build/fperf 10) 2>&1 | grep cpu | awk '{print $9}'
  #  echo "$i done"
  done
done