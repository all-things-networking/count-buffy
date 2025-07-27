set -euo pipefail

BUILD_DIR="cmake-build-debug"

echo > $1
LOG_FILE=$(realpath "$1")

cd $BUILD_DIR

for ((i=10; i<=550; i = i + 50)); do
  ./Single 4 7 1 $i >> "$LOG_FILE"
done