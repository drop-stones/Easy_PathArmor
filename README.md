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
  * CFGのSerialization修正
    * 複数参照に対応
  * CFG作成の修正
    * 複数のreturn先を持つことに対応してなかった!!
    * ベーシックブロックの分割に対応
  * kernel module with Pin
  * Path Verification
  * direct jump, direct call/return Verification
* To do
  * indirect jump Verification
  * indirect call Verification
