# cfg_generation

## To do

* 複数先へのreturnに対応
  * returnをvectorに変更
  * return先も一緒に保存する必要があるかも
  * vectorで順序を保持?
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
      * 方法2: まだSerializeしていないnodeは展開、既出のnodeは参照のみ
         * save: saveしたnodeを記録しておき、既出かどうか確認
         * load: nodeのidが既出かどうか確認
