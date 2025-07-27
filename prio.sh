set -euo pipefail

BUF_SIZE=$1

cd cmake-build-debug
cmake --build . --target Single -- -j 6

./Single 4 7 1 $BUF_SIZE