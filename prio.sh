set -euo pipefail

BUILD_DIR="cmake-build-debug"
TARGET=Prio

cmake --build $BUILD_DIR --target clean
cmake --build $BUILD_DIR --target $TARGET -- -j 6

for i in 10 25 50 75 100 125 150 175 200 250 300 350 400 450 500; do
  echo Starting buf_size=$i
  ./$BUILD_DIR/$TARGET 4 7 1 $i
  echo Done buf_size=$i
done
