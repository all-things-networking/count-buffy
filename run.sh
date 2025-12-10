set -euo pipefail

BUILD_DIR="cmake-build-debug"
TARGET=$1

cmake --build $BUILD_DIR --target $TARGET -- -j 6

for i in 75 100 150; do
  echo "Running for size=$i"
  ./$BUILD_DIR/$TARGET $i
#  if [ -f "wls/$TARGET.${i}.txt" ]; then
#    echo "${i}.txt exists"
#  fi
done