# 分支与循环技术细节

## DragonIR相关
TODO: 整型变量内部转换为布尔类型的值，同样布尔类型的值转换为整型值；非0的int类型为真，否则为0；真为1，否则为0

## ARM32相关

### ARM32对分支选择的翻译
ARM32对DragonIR的翻译相当于一次窥孔优化，将原本的icmp->bc重新解释成cmp->b\<cond\>->b三条指令

### icmp带值
在ARM32中，cmp指令本身是一个不依赖结果寄存器的指令，使用标志位判断是否跳转，而DragonIR中使用布尔值保存icmp结果，这里就产生了冲突。

因此，我们需要使用mov\<cond\>指令将计算结果迁移到寄存器中，将其保存。

需要注意的是，并不是所有情况都要加上这几条指令，如果指令的结果值被使用才调用以上的逻辑。

因此，我们在`Instruction`类中增加方法`isUsed()`进行判断：

```cpp
///
/// @brief 检查指令的结果是否被使用
///
bool Instruction::isUsed()
{
    if (this->uses.size() != 0) {
        return true;
    } else {
        return false;
	}
}
```