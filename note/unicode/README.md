## 文件夹内容介绍
本文件夹下include和lib中文件是[libutf](https://github.com/cls/libutf)中编译的静态库，main.c和utf_new.c文件使用静态库的实验代码。myutf是unicode，utf8，utf16相互转化的代码，使用cmake编译，其中学到的是在add_custom_command中使用COMMAND程序，比如awk，其中涉及到单引号和双引号，无法通过，比如：
```shell
add_custom_command(TARGET utf POST_BUILD
	COMMAND awk -v FS=';' '{ print $1 }' "${CMAKE_SOURCE_DIR}/UnicodeData.txt" > "${CMAKE_BINARY_DIR}/codepoint.txt"
)
```
会报错提示单引号问题，可以使用```bash -c```:
```shell
add_custom_command(TARGET utf POST_BUILD
	COMMAND bash-c "awk -v FS=';' '{ print $1 }' \"${CMAKE_SOURCE_DIR}/UnicodeData.txt\" > \"${CMAKE_BINARY_DIR}/codepoint.txt\""
)
```
不过要注意的是，需要注意转义(escape)符号。  
使用以下命令编译文件：
```shell
gcc -g -Wall -Wextra -pedantic main.c lib/libutf.a -o main -I./include
```
## 使用python获取Code Point, utf8/utf16/utf32编码
```python
# chr(ordinal) -> binary str
# ord(str) -> ordinal
chr(0x132).encode('utf8').hex()
chr(0x132).encode('utf16').hex('-', 2)
chr(0x132).encode('utf_16_le').hex()
chr(0x132).encode('unicode_escape')
print('{0:#x}'.format(ord('\u132')))
```
## UNICODE
- 最早使用ASCII码128(0x7F)标识字符，其中0-0x1F和0x7F为控制字符(总共33个)，不可见。  
- 接着各国开始扩展自己的字符集，最后UNICODE大一统，使用4个字节长度标识全球不同的语言字符，目前实际长度为0x10FFFF，所以在底层表示的时候，比如使用C语言，有使用uint32_t表示的，也有使用int32_t表示的，只要能满足长度范围，使用有符号的，可以使用-1等表示错误；虽然规定了字符，但是字符显示形式由各平台控制，比如各平台的笑脸emoji效果是不一样的；
- 字符使用unicode的code point标识，相当于一个编号(Ordinal)，每个字符的持久化存储，或者传输有多种形式，专业称为编解码，常用的形式有utf8，utf16，utf32等。  
1. utf8是变长的编码格式，使用1-4个字节编码，正常情况下utf8与平台大小端没有关系，但是windows平台某些使用场景依然会进行大小端判断，比如有的文件最前端有字符```0xEF, 0xBB, 0xBF```(刚好是0xFEFF的utf8编码形式)；兼容ASCII码，相对于其他几种编码形式，如果大量使用英文，可以更加节省空间 
2. utf16是变长的编码格式，使用2个或者4个字节编码，编码保存或者传输跟大小端有关系，比如使用```0xFFFE```标识小端(高位字节存储在低位)；使用2字节表示英文，相当于不兼容ASCII
- utf8编码算法

| 码点的位数 | 码点起值	| 码点终值	| 字节序列	| Byte 1 | Byte 2 | Byte 3 | Byte 4 | Byte 5 | Byte 6 |
| :----: | :---- | :---- | :----: | ----  | ----  | ----  | ----  | ----  | ----  |
| 7	   | U+0000 |	U+007F |	1	|0xxxxxxx |
|11	|U+0080|	U+07FF|	2 |	110xxxxx|	10xxxxxx|
|16	|U+0800|	U+FFFF|	3	 |1110xxxx	|10xxxxxx|	10xxxxxx|
|21	|U+10000|	U+1FFFFF|	4	 |11110xxx	|10xxxxxx|	10xxxxxx |	10xxxxxx|
|26	|U+200000|	U+3FFFFFF|	5	 |111110xx|	10xxxxxx|	10xxxxxx|	10xxxxxx|	10xxxxxx|
|31	|U+4000000|	U+7FFFFFFF|	6	 |1111110x|	10xxxxxx|	10xxxxxx	|10xxxxxx|	10xxxxxx|	10xxxxxx|

- utf16编码算法
说这个之前，首先弄清楚一个概念，[0-0x10FFFF总共分成了17个平面(plane)](https://zh.wikipedia.org/wiki/Unicode%E5%AD%97%E7%AC%A6%E5%B9%B3%E9%9D%A2%E6%98%A0%E5%B0%84)，每个平面65536(2的16次方)个代码点，目前只用了少数平面。

| 平面 | 始末字符值 | 中文名称| 英文名称 |
|----|----|:----|:----|
| 0号平面 | U+0000 - U+FFFF | 基本多文种平面 | Basic Multilingual Plane(BMP) |
| 1号平面 | U+10000 - U+1FFFF | 多文种补充平面 | Supplementary Multilingual Plane(SMP) |
| 2号平面 | U+20000 - U+2FFFF | 表意文字补充平面 | Supplementary Ideographic Plane(SIP) |
| 3号平面 | U+30000 - U+3FFFF | 表意文字第三平面 | Tertiary Ideographic Plane(TIP) |
| 4号平面 - 13号平面 | U+40000 - U+DFFFF | （尚未使用） | |
| 14号平面 | U+E0000 - U+EFFFF | 特殊用途补充平面 | Supplementary Special-purpose Plane(SSP) |
| 15号平面 | U+F0000 - U+FFFFF | 保留作为私人使用区（A区） | Private Use Area-A(PUA-A) |
| 16号平面 | U+100000 - U+10FFFFF | 保留作为私人使用区（B区） | Private Use Area-B(PUA-B) |

utf-16使用2个字节(0-0xFFFF)表示代码点，对于基本多平面（BMP）0-0xFFFF的代码点可以直接复写，但是大于0xFFFF的代码点就需要使用额外2个字段表示了，但是怎么表示呢？这里引入了代理对(surrogate pair)的概念，BMP中D800-DFFF不作为码点:
```
D800-DBFF utf-16高半区(high surrogate pair)
DC00-DFFF utf-16低半区(low surrogate pair)
```
```
码位减去 0x10000，得到的值的范围为20比特长的 0...0xFFFFF。
高位的10比特的值（值的范围为 0...0x3FF）被加上 0xD800 得到第一个码元或称作高位代理（high surrogate），值的范围是 0xD800...0xDBFF。由于高位代理比低位代理的值要小，所以为了避免混淆使用，Unicode标准现在称高位代理为前导代理（lead surrogates）。
低位的10比特的值（值的范围也是 0...0x3FF）被加上 0xDC00 得到第二个码元或称作低位代理（low surrogate），现在值的范围是 0xDC00...0xDFFF。由于低位代理比高位代理的值要大，所以为了避免混淆使用，Unicode标准现在称低位代理为后尾代理（trail surrogates）。

以U+10437编码（𐐷）为例:
0x10437 减去 0x10000，结果为0x00437，二进制为 0000 0000 0100 0011 0111
分割它的上10位值和下10位值（使用二进制）：0000 0000 01 和 00 0011 0111
添加 0xD800 到上值，以形成高位：0xD800 + 0x0001 = 0xD801
添加 0xDC00 到下值，以形成低位：0xDC00 + 0x0037 = 0xDC37
```
- Unicode比较
[unicode的数据可以从官方下载](https://unicode.org/Public/UNIDATA/UnicodeData.txt)，下载的文本里面以行为单位，每行格式如下
格式详细解释参考书籍[Unicode Demisify](UnicodeDemystified.pdf)第126页介绍, 总共有15列
```
1. the character's code point value
2. the character's name
3. the character's general category. 告诉我们字符是letter, digit, combining mark and so on
4. the character's combining class. this value is used to order combining marks when converting Unicode text to one of the Unicode normalized forms.
5. Bidrectional category. 书写方向，左边，右边等
6. Decomposition. For decomposing characters, the characters in the decomposition and a tag indicating the decomposition type.
7. Decimal digit value. If the charater can be used as a decimal digit, the numeric value it represents.
8. Digit value. If the character can be used as a digit(decimal or not), the numeric value it represents.
9. Numeric value. If the character can be used alone as a numera, the numeric value it represents.
10. Mirrored. Says whether the character adopts a mirror-image glyph when surrounded by right-to-left text.
11. Unicode 1.0 name.
12. 10646 comment field. 
13. Uppercase mapping.
14. Lowercase mapping.
15. Titlecase(首字符大写) mapping.
```
举例
```
0041;LATIN CAPITAL LETTER A;Lu;0;L;;;;;N;;;;0061;
上行为拉丁字母A, letter uppercase, 对应小写为0061
022C;LATIN CAPITAL LETTER O WITH TILDE AND MACRON;Lu;0;L;00D5 0304;;;;N;;;;022D;
上行022C为letter uppercase，对应的Decomposition为\u00D5\u0304，对应小写为022D
```
- Normalize Form  
可以看出Unicode除了使用单个code point表示字符，有些字符也可以使用组合形式表示，比如\u00D5\u0304组合形式就可以表示\u022C（precomposed character）表示的字符；还有半角逗号','(\u002C)和全角逗号'，'(\uFF0C)。  
上述现象在Unicode中叫做normalize form，分为Canonical composition(decomposition)和Compatible composition(decomposition)，\u022C的Canonical Decomposition是\u00D5\u0304，The compatibility decompositis大致分为16种(第110页)：superscripts and subscripts, Font variants, Circled characters, halfwidth and fullwidth, square fonts and so on，综上所述，有的使用precomposition character，有的使用decomposed characters，还有的使用全角，有的使用半角，这个比较带来困扰。有的编程语言引入了normalize的函数用于处理上述情况，比较类型有NFC、NFD、NFKC、NFKD等。
```python
import normalize from unicodedata
normalize('NFD', '\u022C') == '\u00D5\u0304' # False
[hex(ord(i)) for i in normalize('NFD', '\u022C')] # ['0x4f', '0x303', '0x304']
# \u00D5也可以分解为\u004F\u0303, 所以normalize使用NFD是分解到底
# 还可以使用ascii查看str，显示ascii码，不能显示的使用codepoint显示
ascii(normalize('NFD', '\u022C'))

normalize('NFKD', '，') == ',' # True
```
```shell
# 查看UnicodeData.txt数字列
awk 'BEGIN{FS=OFS=";"} { if($7!=""||$8!=""||$9!="") {printf "%5s,%5s,%5s,%5s\n", $1, $7, $8, $9}}' UnicodeData.txt
```
