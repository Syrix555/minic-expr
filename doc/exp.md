# 注意事项
return返回值为空的情况；
全局变量赋初值——只写了初始值为常数的情况，没写数组赋初值	初值为0也应当放在.bss段内
if条件中数组也算单变量
逻辑非考虑变量为数组的情况
if,while条件中计算式结果也是单变量

while中嵌套多个Block导致标签无法继承下去
break, continue直接是循环体的情况时的标签继承