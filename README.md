## 关于
本工程fork自官方的mujs库，主要用于学习js引擎相关内容，引入libevent和libcurl，实现了简单的timer、xhr、queueMicrotask等接口, 例子可以参见 [xhr.js](./note/demo/xhr.js), [microtask.js](./note/demo/microtask.js)等  
根目录makefile有更新，主要是添加相关库依赖
## 执行
```shell
sudo apt install libevent-dev curl  # 也可以自行编译相关的库
make nuke # 主要是重新生成one.c文件
make debug
./build/debug/mujs ./note/demo/xhr.js
```
## 接口
1. timer
接口主要代码见 [jstimer.c](./jstimer.c)，主要实现的接口有
- setTimeout
- setInterval
- clearTimeout
- clearInterval  
**需要注意的是libevent使用最小堆方式存放timer，导致部分情况下执行顺序问题，见[例子](./note/demo/libuv/simple.c)，为了实现有序，对于相同时间事件在代码中添加了n微秒(n为出现次数)**
2. XHRHttpRequest
接口主要代码见 [jsxhr.c](./jsxhr.c)，没有实现event相关，主要实现的接口有
- open
- onload
- send
- timeout
- ontimeout
- response
- responseType
- onprogress 虽然没有提供event参数，但是lengthComputable, loaded, total值已经实现
- new XHRHttpRequest()
3. queueMicrotask
接口见代码[jstask.c](./jstask.c), 例子见[microtask.js](./note/demo/microtask.js)

## 作者的README
MuJS: an embeddable Javascript interpreter in C.

ABOUT

MuJS is a lightweight Javascript interpreter designed for embedding in
other software to extend them with scripting capabilities.

LICENSE

MuJS is Copyright 2013-2017 Artifex Software, Inc.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

The software is provided "as is" and the author disclaims all warranties with
regard to this software including all implied warranties of merchantability
and fitness. In no event shall the author be liable for any special, direct,
indirect, or consequential damages or any damages whatsoever resulting from
loss of use, data or profits, whether in an action of contract, negligence
or other tortious action, arising out of or in connection with the use or
performance of this software.

COMPILING

If you are building from source you can either use the provided Unix Makefile:

	make release

Or compile the source with your preferred compiler:

	cc -O2 -c one.c -o libmujs.o

INSTALLING

To install the MuJS command line interpreter, static library and header file:

	make prefix=/usr/local install

DOWNLOAD

The latest development source is available directly from the git repository:

	git clone http://git.ghostscript.com/mujs.git

REPORTING BUGS AND PROBLEMS

Report bugs on the ghostscript bugzilla, with MuJS as the selected component.

	http://bugs.ghostscript.com/

The MuJS developers hang out on IRC in the #mupdf channel on irc.freenode.net.
