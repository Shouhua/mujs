## openssl
主要使用openssl库的各种demo文件,可能比较偏底层，各种资料比较少，官网的文档门槛较高
## bio and evp
bio主要是基本的操作，比如文件操作
evp抽象的高级操作，比如加解密等
## tcp连接使用openssl库流程
1. create context
```c
SSL_CTX_new(TLS_server_method());
```
2. kinds of settings
```c
SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH);
SSL_CTX_set_keylog_callback(ctx, SSL_CTX_keylog_cb_func_cb);
SSL_CTX_set_ex_data(ctx, custom_object); //设置自定义数据，也可以绑定到ssl对象上
```
3. add server ca and private key
```c
SSL_CTX_use_certificate_file(ctx, ca_path, SSL_FILETYPE_PEM)
SSL_CTX_use_PrivateKey_file(ctx, private_key_path, SSL_FILETYPE_PEM)
SSL_CTX_check_private_key(ctx)
```
4. new ssl object after get client socket
```c
SSL_new(ctx)
SSL_set_fd(ssl, client_socket)
SSL_accept(ssl)
SSL_read(ssl, buf, buf_len)
SSL_write(ssl, buf, buf_len)
```
5. lastly, close resources
```c
SSL_shutdown(ssl)
SSL_free(ssl)
SSL_CTX_free(ctx)
```