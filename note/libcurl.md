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
curl_easy_init()
curl_easy_setopt(easy_handle, CURLOPT_URL, "http://www.url.com");
curl_easy_perform(easy_handle)
curl_easy_getinfo(CURL *curl, CURL_INFO_***, &value)
```
- multi handle