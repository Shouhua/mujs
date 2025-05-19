## 2024-12-07
使用ingress-nginx时，注意本地proxy和all_proxy等会干扰curl请求
```bash
# -b 使用cookies.txt里面的内容作为cookie发送, 也可以使用key=value的形式传递键值对
# -c 保存cookie到文件 
curl -b cookies.txt -c cookies.txt --noproxy '*' --resolve kubia.example.com:80:127.0.0.1 http://kubia.example.com
```

## 2024-11-30
### ffmpeg hello/remuxing
avformat_open_input就是初始化了avformat context

### vscode配置lldb
https://code.visualstudio.com/docs/cpp/lldb-mi
默认情况下，mac平台使用gdb，各种codesign等，搞不定，还是使用平台自带的clang和lldb，但是默认情况下lldb-mi没有显示的包括进来，所以在设置的时候要设置全路径，另外可以在lldb前执行task任务，可以执行编译命令或者执行make执行
```json
// launch.json 配置
{
    "name": "ffmpeg-hello",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/note/ffmpeg/hello",
    "args": ["bbb_sunflower_1080p_30fps_normal.mp4"],
    "stopAtEntry": true,
    "cwd": "${workspaceFolder}/note/ffmpeg/",
    "environment": [],
    "externalConsole": false,
    "targetArchitecture": "x64",
    "MIMode": "gdb",
    "miDebuggerPath": "/Users/pengshouhua/.vscode/extensions/ms-vscode.cpptools-1.22.11-darwin-x64/debugAdapters/lldb-mi/bin/lldb-mi",
    "preLaunchTask": "build ffmpeg hello" // 指定在调试前执行的任务
}

// task.json
{
    "label": "build ffmpeg hello",
    "type": "shell",
    "options": {
        "cwd": "${workspaceFolder}/note/ffmpeg" // 指定当前执行的目录
    },
    "command": "make",
    "args": [
        "hello" // make hello
    ],
    "group": {
        "kind": "build",
        "isDefault": true
    },
    "problemMatcher": ["$gcc"],
    "detail": "Build the project using Make"
}
```

## 2024-11-29
### ffmpeg录音
```bash
ffmpeg -hide_banner -list_devices true -f avfoundation -i "" # 列出系统支持的视频和音频源
ffmpeg -y -hide_banner -f avfoundation -i ":1" -t 30 out.wav
ffmpeg -y -hide_banner -f avfoundation -capture_cursor 1 -i "3:1" -r:v 30 out.mp4 # 录屏包括鼠标
ffplay -autoexit out.wav

ffmpeg -h -encoder=libopus

ffplay -help demuxer=f32le
ffplay -hide_banner -autoexit -f s16le -sample_rate 44100 -ch_layout 1 -i audio.pcm

ffmpeg -f f32le -i audio.pcm -f s16le -sample_rate 44100 -ac 2 audio_resample.pcm
ffplay -hide_banner -autoexit -f s16le -sample_rate 44100 -ch_layout stereo audio_resample.pcm
```

## 2024-11-14
```js
JSON.parse(document.querySelector('.dplayer').attributes['data-config'].textContent).video.url
```
1. get html
2. get m3u8 url
3. get video by ffmpeg
TODO: 处理一个页面多个视频 20231, 114286
```bash
ffmpeg -protocol_whitelist file,http,https,tcp,tls,crypto -i "https://m3u8.url" -acodec copy -vcodec copy out.mp4
```

## 2024-05-15
### Makefile
- `@D` 表示目标文件的目录
```bash
build/debug/mujs: one.c
    mkdir -p $(@D)
```

## 2023-06-25
- [GDB Convenience Vars and Functions](https://sourceware.org/gdb/onlinedocs/gdb/Convenience-Vars.html#Convenience-Vars)
```shell
# gdb set conditional breakpoint
b jsstate.c:109 if $_streq(filename, "if.js")
```
- 优化点：if/else if/else太多opcode，是不是可以在编译阶段判断语言丢弃不用的分支?
- if的opcode见文件[if.js](./js/if.js)
- -0和+0，浮点数标准，30.0/0 VS -30.0/0
```c
/* math.h 提供了相关的定义和宏*/
NAN;
INFINITY;
int fpclassify(x);
int isfinite(x);
int isnan(x);
int isinf(x);
static __inline int signbit(double x) { __int64 i; memcpy(&i, &x, 8); return i>>63; }
```
## 2023-06-20
- ptrace和debugger  
1. [调试器工作原理](https://abcdxyzk.github.io/blog/2013/11/29/debug-debuger-1/)  
2. [How Debuggers Works](https://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1)
- 可变参数函数
[va_func.c](./c/va_func.c)
- 新的字体
[Iosevka](https://github.com/be5invis/Iosevka)
## 2023-06-19
- 源码中Object初始对象新建过程：
1. 新建Object prototype object
2. 添加prototype方法，比如Object.prototype.hasOwnProperty
3. 新建Object，填充Object的prototype对象、constructor、cfun(不知道叫啥，直接使用对象初始化时调用)
4. 添加Object方法，比如Object.defineProperty
5. 定义全局对象“Object”，并且关联到前面的Object对象  
**定义constructor时候，签名如下，其中cfun用于直接引用时调用，比如Object(3)，ccon用于使用new时调用，比如new Object(3)**
```c
void js_newcconstructor(js_State *J, js_CFunction cfun, js_CFunction ccon, const char *name, int length);
```
- ECDSA, ECDH
Ecliptic Curve Digital Signed Algorithm
Ecliptic Curve Diffie-Hellman
- seek, ftell VS stat
前者属于标准库，用于普通文件，不适用于如管道、设备文件等；后者能提供除大小外的详细信息
- readline库
```shell
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#define HISTORY_FILE ".myapp_history"

int main() {
    char* input;
    
    // 加载历史记录文件
    char history_file_path[256];
    snprintf(history_file_path, sizeof(history_file_path), "%s/%s", getenv("HOME"), HISTORY_FILE);
    using_history();
    read_history(history_file_path);

    while ((input = readline("> ")) != NULL) {
        if (input[0] != '\0') {
            // 处理输入的命令
            printf("Input: %s\n", input);
        }

        // 将命令行添加到历史记录并写入文件
        add_history(input);
        write_history(history_file_path);

        free(input);
    }

    return 0;
}
```
- inode、软链接、硬链接
inode属于文件系统概念，文件描述符表属于进程概念，信息存储在PCB中，每个进程有一份数据
1. 创建软链接时，操作系统会创建一个新的普通文件，并将该文件的内容设置为指向原始文件或目录的路径。
文件系统会分配一个新的inode给这个软链接文件，并在该inode的数据区域存储软链接的内容。
当访问软链接时，操作系统读取软链接的内容并跳转到对应路径的文件或目录。
2. 创建硬链接时，操作系统在同一个目录下为目标文件或目录创建一个新的目录项，与原始文件或目录共享相同的inode。
目录项包含文件名和inode号码，通过目录项来查找和访问文件。
即使创建多个硬链接，它们在文件系统中仍然只有一个inode，因此它们共享相同的元数据和数据块。
- gdb的mi模式
```shell
gdb -i=mi ./test
# a. 发送 -break-insert main 命令设置在 main 函数上的断点。
# b. 发送 -exec-run 命令运行程序。
# c. 接收 GDB 返回的响应，其中包含断点命中的信息和程序输出。
# d. 可以使用其他的 MI 命令，如 -data-evaluate-expression 来评估表达式，或者 -stack-list-variables 来列出当前栈帧中的变量。
```
## 2023-05-25
- 问题：gitlab ci中使用nvm切换报错，nvm command not found  
我使用ssh登录后虚拟机后，使用```sudo -i -u gitlab-runner```切换到gitlab-runner用户，安装nvm，本地切换node环境很正常  
gitlab ci使用用户gitlab-runner用ssh登录系统，在shell环境下执行ci脚本，但是出现上述问题  
仔细查看nvm文档，发现需要将nvm环境脚本添加到~/.bashrc文件中，我仔细check，发现已经添加了，很迷惑！！！
最终答案是使用ssh登录时，只是执行了/etc/profile和gitlab-runner用户目录下的~/.bash_profile，但是咱们~/.bash_profile里面没有内容，导致找不到nvm命令；一般情况下，~/.bash_profile文件中会询问是否存在~/.bashrc，存在就加载，比如：
```shell
[[ -f "~/.bashrc" ]] && . ~/.bashrc
```
- login shell, nonlogin shell, script shell
1. login shell是咱们登录系统时使用的shell，比如使用ssh登录时候，依次会调用
```shell
/etc/profile
~/.bash_profile # 以下三个遇到就执行，后面不执行
# ~/.bash_login
# ~/.profile
```
一般情况下，~/.bash_profile文件中会询问是否存在~/.bashrc，存在就加载，比如：
```shell
[[ -f "~/.bashrc" ]] && . ~/.bashrc
```
2. nonlogin shell是咱们登录系统后，在GUI中打开terminal，这个时候就是这种情况；还有就是使用bash也是此类情况，这种情况会调用~/.bashrc
3. script shell就是在脚本中运行shell命令，这个只会继承当前shell**导出的**环境变量
## 2023-05-23
使用cmake编译系统，详情见CMakeLists.txt，目前暂时支持debug
## 2023-05-22
- Unicode编码: [utf8](https://zh.wikipedia.org/wiki/UTF-8), [utf16](https://zh.wikipedia.org/wiki/UTF-16), ucs2
- [Unicode等价性](https://zh.wikipedia.org/zh-cn/Unicode%E7%AD%89%E5%83%B9%E6%80%A7)：NFC，NFD, NKFC,NKFD
- [字节序](https://zh.wikipedia.org/zh-cn/%E4%BD%8D%E5%85%83%E7%B5%84%E9%A0%86%E5%BA%8F%E8%A8%98%E8%99%9F): 0xFFFE是不存在的字符codepoint，原始的标记字符是0xFEFF，可以使用这个判断大小端，比如intel小端存储为0xFF 0xFE；utf16是跟系统大小端相关，但是utf8是不相关的，但是有的系统还是在文件头部添加了标识，比如csv使用excel格式打开时，就需要在头部添加编码，在utf8文件BOM是0xFEFF的utf8编码0xEF 0xBB 0xBF
## 2023-05-17
- this  
jsrun.c->jsR_run->OP_THIS，可以看到this代表执行时调用object
- JS_CCFUNCTION/JS_CFUNCTION  
JS_CCFUNCTION，c语言的函数，调用时直接调用c code，c code使用时注意传参和返参使用方式，比如js_gettop等; 区别举个例子比如```jserror.c->Ep_toString```，默认是```Error.prototype.toString```的c执行函数，使用```main.c->js_dostring(J, stacktrace_js)```使用js修改```Error.prototype.toString```后，变成了JS_CFUNCTION
JS_CFUNCTION，js的function，运行时已经时opcode了，同上面举得例子
## 2023-05-11
1. cmake execute_process运行在generation阶段(cmake命令执行)，add_custom_command运行在compilation阶段(make命令执行); 比如在build阶段；根据某些文件生成中间文件，可以使用add_custom_command，依赖文件没有更新，就不会执行；另外后者可以选择在何时，比如build前后执行;前者更偏向于每次都执行的命令，比如根据git tag获取版本，两者的区别可以慢慢领会。
2. 两者使用可以参见根目录下的CMakeLists.txt中生成utfdata.h头文件，开始使用execute_process在配置阶段，每次都会重新生成下；后面改成使用add_custom_command在make执行阶段生成，这样在文件修改后不会每次生成。起始感觉都一样，因为cmake命令也不是每次执行，但是意义有些不一样，另外使用execute_process可以输出为变量，见version填充。
3. add_custom_command中的COMMAND参数一般使用```${CMAKE_COMMAND} -E echo "Hello, world!"```，这样会将后面的命令作为cmake脚本执行，如果不添加```${CMAKE_COMMAND} -E```会直接在机器上执行
## 2023-05-10
1. (-fsanitize=address)[https://github.com/hellogcc/100-gcc-tips/blob/master/src/address-sanitizer.md], 运行时检测比如数组越界访问等内存问题
2. (-fno-omit-frame-pointer/-fomit-frame-pointer)[https://breezetemple.github.io/2020/02/26/gcc-options-fomit-frame-pointer/], 编译是否保存stack frame用于调试调用栈, **使用-O0的时候会保存调试信息，及默认-fno-omit-frame-pointer**
3. cmake set/unset
详情见(set官方文档)[https://cmake.org/cmake/help/latest/command/set.html]
- set normal vairable
```set(<variable> <value>... [PARENT_SCOPE])``` 这种情况下相当于局部变量
- set cache entry
```set(<variable> <value>... CACHE <type> <docstring> [FORCE])``` 设置全局变量，变量将放在CMakeCache.txt文件中，如果要覆盖，需要设置FORCE参数，cmake自有变量会默认放在cache文件中，比如CMAKE_BUILD_TYPE，如果不设置，cache文件中值为空；另外，使用-DHello=world也会放在cache文件中
- set environment variable
```set(ENV{<variable>} [<value>])```
## 2023-04-12
1. c函数中获取js调用传递函数参数时的argc和argv，注册时候要注意写入最小参数个数，这个在调用时会补全，如果写的时全量的参数个数，永远补全就不能获取动态argc了，详细可以参考Xp_open函数获取第三个async参数。argc确定后，不同类型的argv使用不同的方法获取，比如boolean使用js_toboolean(J, -1)
2. 书籍和有用的website
flex和bison
lua设计与实现
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
二义性，相同的输入可能被多个不同的模式匹配，比如1-2+3,可以是1-（2+3），也可以是（1-2）+3, 使用以下规则解决
- 词法分析器匹配输入时匹配尽可能多的字符串
- 如果两个模式可以匹配的话，匹配在程序中更早出现的模式
### bison处理优先级和结合性
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
## 2023-06-06
[gcc -l参数为什么需要放在输入文件后面](https://stackoverflow.com/questions/11893996/why-does-the-order-of-l-option-in-gcc-matter)，因为先扫描输入文件，找到依赖项，然后碰到库文件，会将库文件中对应文件加载，后面再扫描遇到依赖这个库文件的输入文件，不会回溯，所以直接将库文件都放在输入文件后面。
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
utf-32, utf-16, utf-8先后出现
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
普通的从0开始连续的数组是Dense Array，比如```var a=[1,2,3]```，比如```let a = []; a[100]=1;```这种就是sparse array。在```jsi.h```定义```js_Object```中使用count跟这个主题有关系
### [object transient](https://fulmicoton.com/posts/transient/)
表示序列化时候比如toJSON时候是否显示
### function调用
先this，再params, 可以使用gettop(J)获取函数个数当然还要减去this
### 字典（hashmap）

### Closure

### shell中函数return值是$?值，可以在调用函数后判断$?
