# Easy_PathArmor

## 説明

* cfg_generation: Tritonを用いて、CFGを作成
* kernel_module: Pinを用いて、Control Flowを監視
* Test: テスト用ディレクトリ

## How to Compile
```bash
PathArmor$ make
```
## 進捗

* Done
  * CFG作成 with Triton
  * CFGのSerialization

* To do
  * kernel module with Pin
  * Path Verification
