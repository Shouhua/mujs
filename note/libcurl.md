## [libcurl](https://curl.se/libcurl/using/) 主要参考文档：https://everything.curl.dev/libcurl
### 安装
libcurl中ssl依赖支持多种，比如openssl、GnuTLS、NSS，选择一个安装，比如
```shell
sudo apt install libcurl4-openssl-dev
```
### 使用
libcurl常用的有easy handle和multi handle，前者比较简单直接，使用同步方式，后者可以操作方式计较多，后面详细介绍，另外官方还有libcurl-share方式没有接触。<br/>
- easy handle
```c
curl_global_init();
curl_easy_init();
curl_easy_setopt(easy_handle, CURLOPT_URL, "http://www.url.com");
curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION, header_cb); // 接收header数据回调
curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, ctx); // 设置回调函数中的自定义数据
curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_cb); // 接收body数据回调
curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, ctx); // 设置回调函数中的自定义数据
curl_easy_setopt(easy_handle, CURLOPT_XFERINFOFUNCTION, progress_cb); // 进度回调函数
curl_easy_setopt(easy_handle, CURLOPT_XFERINFODATA, ctx); // 设置进度回调函数中的自定义数据

curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L); // 开启进度监控
CURLcode code = curl_easy_perform(easy_handle); // 开始执行
if(code == CURLE_OK)
	// 处理收到的数据
	// 使用curl_easy_getinfo(CURL *curl, CURL_INFO_***, &value)获取额外信息
curl_easy_cleanup(easy_handle);
curl_global_cleanup();
```
- multi handle
multi handle稍微复杂点，可以参考demo中的[curl_multi.c](./demo/curl_multi.c)，还有引入第三份event库，比如libevent实现单线程多路复用实现同时下载多个transfer，参考[multi-event.c](./demo/multi-event.c), [curl_libevent.c](./demo/curl_libevent.c)<br/>
需要注意的点是：
1. 不管是引入disanfangevent库，还是使用libcurl的multi handle，都是要跟libcurl的socket和timeout通信
2. 可以想象成libcurl multi有自己的event loop，注册socket和timeout函数，发生event时，转化成libevent的事件和timeout，反向也是一样