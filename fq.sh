set -euo pipefail

BUILD_DIR="cmake-build-debug"
TARGET=FQ

cmake --build $BUILD_DIR --target clean
cmake --build $BUILD_DIR --target $TARGET -- -j 6

#for i in 10 20 25 30 40 50 75 100; do
#for i in 250 350 450; do
#for i in 550 600 700 800 900; do
#for i in 10 20 25 30 40 50 75 100 125 150 175 200 300 400 500; do
for i in 400 500; do
  echo Starting buf_size=$i
  ./$BUILD_DIR/$TARGET $i
  echo Done buf_size=$i
done
