# CFG_Generation with Triton

## 説明
Tritonのエミュレーションモードを用いて、Control Flow Graphを作成する。</ br>
各エミュレーションにて、CFGのノード・エッジの追加と、未探索なパスへ到達するための入力を計算する。</ br>
新しい入力が計算されなくなるまでエミュレーションを繰り返すことで、到達可能な全パスを探索する。

## How to Compile
```bash
PathArmor/Triton$ make
```

## Usage
```C++
#include "cfg_generation.h"

int main (int argc, char *argv []) {
  ...
  cfg_init ();
  cfg_generation (bin_file, config_file);
  ...
  cfg_fini ();
}
```
