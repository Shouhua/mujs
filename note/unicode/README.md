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