set -euo pipefail

BUILD_DIR="cmake-build-debug"

cmake --build $BUILD_DIR --target LoomMem -- -j 6

#for i in 10 15 20 25 30 35 40 45 50; do
for i in 50 100 200 300 400 500 600 700 800 900; do
  echo Starting buf_size=$i
  ./$BUILD_DIR/LoomMem $i 251
done