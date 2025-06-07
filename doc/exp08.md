# 数组实现技术细节

## exp07遗留问题
stmt:T_RETURN expr T_SEMICOLON 中expr应该为可选?，需要后续修改ASTVisitor实现

## 错误处理
暂且忽略错误处理，后续完成

Next Step: 修改左值与函数形参中对维度的处理（有可能也需要修改IR实现），将维度信息从id_node转移到一个新节点，这个新节点保存id_node与维度信息。