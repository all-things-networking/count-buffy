set -euo pipefail

BUILD_DIR="cmake-build-debug"
TARGET=Single

cmake --build $BUILD_DIR --target $TARGET -- -j 6

for i in 10 15 20 25 30 35 40 45; do
#for i in 10 25 50 75 100 125 150 175 200 250 300 350 400 450; do
#for i in 50 100 200 300 400 500 600 700 800 900; do
  echo Starting buf_size=$i
#  ./$BUILD_DIR/$TARGET 4 7 1 $i
#  ./$BUILD_DIR/$TARGET 5 10 1 $i
  ./$BUILD_DIR/LoomMem $i 251
done