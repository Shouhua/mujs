n = Object(3)
var a = 1
var a = 2
console.log(a)

/**
 * -exec p *(js_Value *)(J->stack+1)
 * 
 * function code(len: 44):
0 lastline
13 undefine

1 lastline(node->line)
6 op_integer
32769 (a=1)
1 lastline(node->line)
20 op_setlocal // 将stack(-1)的值设置到vertab[1]
1 (vartab index)
1 lastline(node->line)
0 op_pop

2 lastline
6 op_integer
32770 (a=2)
2 lastline
20 op_setlocal
1 (vartab index)
2 lastline
0 op_pop

3 lastline
0 op_pop

3 lastline
23 op_getvar(console,因为在vartab中找不到）
6116 (console string addr, 存储的是console字符的地址，分为4个字节)
21852
21845
0 // js_hasvar函数调用时会将console对象压栈

3 lastline
1 op_dup

3 lastline
33 op_getprop_s
6164 (log string addr 以下4个)
21852
21845
0
3 lastline
3 op_rot2  (console和log调换，相当于将console作为this对象入栈)

3 lastline (函数参数 a)
19 op_getlocal
1 a在vartab中index

3 lastline
41 op_call
1 函数参数个数

3 lastline
84 op_return
 */