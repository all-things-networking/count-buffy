# Installation

### Install Z3
```shell
sudo apt update && sudo apt install build-essential
```

Download and install Z3 v4.8.11 [link](https://github.com/Z3Prover/z3/releases/tag/z3-4.8.11).

### Build 
Target is one of the following:
- `RR`
- `Prio`
- `Loom`
- `FQ`
- `Leaf`

```shell
cmake 
cmake --build . --target $TARGET -- -j6
```
### Run

```shell
./$TARGET $BUF_SIZE
```

