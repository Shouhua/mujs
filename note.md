### setjmp/longjmp
c使用setjmp/longjmp模拟try...catch, 例子可见example中的jmp.c
### [Flexible Array Memeber](https://zh.wikipedia.org/wiki/%E7%81%B5%E6%B4%BB%E6%95%B0%E7%BB%84%E7%B1%BB%E5%9E%8B)
比如jsproperty.c中使用的struct类型```js_Property```，初始状态不知道字符长度，使用```char name[1]```代替，在运行时知道长度后使用memory alloc函数正确分配
### GCC no return优化
某些函数确认是没有返回值时，可以告诉编译器，编译器编译时候可以进行优化
```
__attribute__((no_return))
```
### PRINTLIKE
GCC可以帮忙检查类似printf函数的fmt和args类型，详见mujs.h中的定义
## Unicode
将所有文字使用0-4个字节的code point表示，好比是使用一个数字表示字符</br>
- utf(Unicode Transfer Format)，但是我们要在网络间传输或者保存字符，出现很多unicode的编码方式，比如utf-8, utf-16等</br>
- utf-8, 变长编码方式，使用1-4个字节表示，没有字节序考虑，因为需要每次解析一个字节，兼容ASCII
- utf-16, 变长编码方式，两个字节为一个单位，所以会涉及到大小端问题，但是多数汉字使用两个字节就能表示了，utf-8需要使用三个字节，不兼容ASCII，目前Java，c#，js等使用这种编码方式
- BOM(Byte Order Mark)，utf-8不用考虑字节序，但是在windows中还是会添加前缀，比如postgresql导出的csv使用execl打开必须添加BOM

### [词法文法](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Lexical_grammar)
### GC
几条固定的GC链表，比如Env，ast，string等，里面有gcnext，新建时候不断前插
### js_Value
- stack item类型，其中存储字符串，如果字符串长度小于16，使用shrstr存储，如果大于使用引用类型
### js_String
使用上面提到的Flexible Array Memeber
### js_pushtrace
每次在使用调用function的时候会将file name，function name, line记录到trace堆栈中
### precedence
TODO 比如1+2*3，是怎么运行的
### [Sparse Array and Dense Array](https://www.freecodecamp.org/news/sparse-and-dense-arrays-in-javascript/)
普通的从0开始连续的数组是Dense Array，比如```var a=[1,2,3]```，比如```let a = []; a[100]=1;```这种就是sparse array。在```jsi.h```定义```js_Object```种使用count跟这个主题有关系
### [object transient](https://fulmicoton.com/posts/transient/)
表示序列化时候比如toJSON时候是否显示
### function调用
先this，再params
### 字典（hashmap）

### Closure

### shell中函数return值是$?值，可以在调用函数后判断$?
