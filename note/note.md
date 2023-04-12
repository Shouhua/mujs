## 2023-04-12
c函数中获取js调用时的argc和argv，注册时候要注意写入最小参数个数，这个在调用时会补全，如果写的时全量的参数个数，永远补全就不能获取动态argc了，详细可以参考Xp_open函数获取第三个async参数。  
argc确定后，不同类型的argv使用不同的方法获取，比如boolean使用js_toboolean(J, -1)
## 2023-04-11
Opaque Pointers, 比如mujs.h中的js_State, 在业务代码中使用时，是不知道她的内部细节的
## 2023-03-28
### String Intern
mujs AA tree<br/>
hashmap
https://arpitbhayani.me/blogs/string-interning
http://luajit.io/posts/luajit-string-interning/#check-the-source-code
http://lua-users.org/wiki/SpeedingUpStrings
### RDP(递归下降解析)不能使用左递归，使用BNF表达式解决优先级(procedence)，结合性(associative)怎么解决的呢,没有使用左递归或者右递归，根据规则，按照低优先级嵌套在高优先级中递归下降
### (AVL树)[https://www.bilibili.com/video/BV1La411k7vm/?spm_id_from=333.337.search-card.all.click&vd_source=c27240cee6f5eadff60ab6e2661ca959]
### 注意点
词法解析：左递归，右递归，使用的方法，LL(k), LALR(k)，优先级，结合性
语义解析：作用域scope，生存期lifetime
### function成为一等公民，可以被复制给变量，可以当成参数传递给另外一个函数等，**将函数涉及成类型就可以做到这些，进而成为所谓的一等公民**
### 静态作用域，动态作用域
大部分语言是静态作用域，在编译的时候变量的引用就确立了，在运行时候不会根据当前环境提供的变量引用，比如闭包里面的返回函数使用定义时父函数里面的变量引用，不会使用新的<br/>
shell脚本语言时动态作用域，这个在写脚本的时候要注意，所以赋值出现很多种类
## 2023-03-24
### gcc -pedantic -Wall -Wextra
-pedantic表示尽可能遵守C标准，但是并不能完全保证，所以可以添加下后面2个flag(pedantic英文为学究的，迂腐的)
### Flex处理二义性
二义性，相同的sh可能被多个不同的模式匹配，使用以下规则解决
- 词法分析器匹配输入时匹配尽可能多的字符串
- 如果两个模式可以匹配的话，匹配在程序中更早出现的模式
优先级和结合性(precedence and associativity), 以下是显式定义
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS
上面每个声明都定义了一种优先级，%left、%right和%noassoc的出现顺序决定了由低到高的优先级顺序。还有隐式定义优先级，通过将*和/放在规则后面位置匹配，如计算器例子那样
yyrestart() 重新开始lex，比如处理多个文件时候，每个文件重新开始
输入管理的三个层次：
- 是指yyin来读取所需文件(yyin=fopen(fiel, 'r'))
- 创建并使用YY_BUFFER_STATE输入缓冲区
- 重定义YY_INPUT(input()和unput())
输出管理
- #define ECHO fwrite(yytext, yyleng, 1, yyout)
%option noyywrap
%option nodefault 不要添加默认规则，当输入无法被给定规则完全匹配时，词法分析器给出错误
%option yylineno 告诉flex，定义一个名为yylineno的整型变量保存当前行号
%option case-insensitive 要求flex生成一个大小写无关的词法分析器，yytext中被匹配字符串不会改变她们的大小写
### Bison
在输入中出现并且被词法分析器返回的符号是终结符或记号（一般大写），而规则左边的语法符号是非终结符，一般使用小写<br/>
移进与规约(shift and reduce)，比如fre=12+13,先将字符串shift进堆栈
fred
fred=
fred=12
fred=12+
fred=12+13
这时她可以归约规则expression:NUMBER+NUMBER，所以她可以从堆栈中弹出12、加号和13，并把他们替换为expression：
fred = expression
现在它又可以归约规则statement: NAME = expression，所以它弹出fred、=和expression替换为statement，每个归约的规则可以执行相应的action
## 2023-03-22
### Parser
使用RDP(Recursive Descentant Parse)，使用的BNF模型大概率是[这个文档](https://tomcopeland.blogs.com/EcmaScript.html#prod3)或者这个[本地文件](bnfforjavascript.html)
## 2023-03-21
```
2 lastline
81 jump (codelen=34)
0（smtfor完成时候替换成当前的inst=76） (args)  （设置target（for语句）的jumps=jump（inst=34 addr， ）
```
1. 分支跳转，比如break，continue等，栈情况
2. GC
新建对象会使用jsV_newobject, 会将当前object对象放到J->gcobj链表中
## 2023-03-20
### variable shadowing
本地变量覆盖全局变量, 但是在jscompile.c->cvardecs中处理inner function不懂跟这个有什么关系
### addlocal函数
reuse使用场景
```js
var a = 1
var a = 2
// 这里使用a还是为2, 可以重复声明，赋值还是有效
```
### lightweight函数
lightweight函数表示内部函数，比如function里面的函数，区别是lightweight=1参数放在stack上，否则放在当前环境中
## 2023-03-19
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
