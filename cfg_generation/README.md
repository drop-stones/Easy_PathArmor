# cfg_generation

## Done

* 複数先へのreturnに対応
  * <returnアドレス, return_edge>のペアで保持
  * 直近のcallは最後の要素
* 後々に分解できるような構造に変更
  * <callアドレス, call_edge>のペアで保持
  * 同じBBから複数回呼ばれることへの対応
    * 呼ばれる毎にペアを保存
* Serializationの修正
  * 複数からの参照に対応
    * 参照のみをSerializeする
      * 方法1: edgeは参照のみを持つ
         * save: edgeは参照のみ書き込み
         * load: はじめに全てのnodeをロードした後、edgeを張り替える
      * <採用>方法2: まだSerializeしていないnodeは展開、既出のnodeは参照のみ
         * save: saveしたnodeを記録しておき、既出かどうか確認
         * load: nodeのidが既出かどうか確認
* ベーシックブロックの分割
  * cfg_divide_node ()
* 走査 (find function)
  * 走査済みベーシックブロックを保存してループに対処 (searched_set)
* call_edges/return_edgesのデータ構造を変更
  * call: map <addr, set <cfg_node> >に変更
  * return: map <addr, cfg_node>に変更
* call edge/return edgeの同じ要素追加への対処 (for, whileなど)

## To do

* true\_edgeを set <cfg\_node> へ変更 (indirect edgeのとき、複数の飛び先が考えられるため)
* Path Verificationの実装
