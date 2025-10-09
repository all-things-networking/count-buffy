set -euo pipefail

BUILD_DIR="cmake-build-debug"
TARGET=$1

cmake --build $BUILD_DIR --target $TARGET -- -j 6

for i in {10..500}; do
  if [ -f "wls/$TARGET.${i}.txt" ]; then
    echo "${i}.txt exists"
    ./$BUILD_DIR/$TARGET $i
  fi
done