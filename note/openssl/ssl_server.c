#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1024
#define MASTER_KEY "master_key.txt"
#define KEY_LOG "key_log.txt"
#define SERVER_CA "ca_cert.pem"
#define SERVER_KEY "private_key.pem"

void SSL_CTX_keylog_cb_func_cb(const SSL *ssl, const char *line)
{
	FILE  * fp;
	SSL_CTX *ctx = SSL_get_SSL_CTX(ssl);
	char *key_log_path = SSL_CTX_get_ex_data(ctx, 0);

#ifdef DEBUG
	printf("key log路径：%s\n", key_log_path);
#endif

    fp = fopen(key_log_path, "w");
    if (fp == NULL)
    {
        printf("Failed to create log file\n");
    }
    fprintf(fp, "%s\n", line);
    fclose(fp);
}

int main(int argc, char **argv)
{
	int sockfd, new_fd, fd;
	socklen_t len;
	struct sockaddr_in my_addr, their_addr;
	unsigned int myport, lisnum;
	char buf[MAXBUF + 1];
	SSL_CTX *ctx;
	char pwd[100];

	char key_log_path[512], master_key_path[512], server_ca_path[512], server_key_path[512];
	getcwd(pwd, 100);
	sprintf(key_log_path, "%s/%s", pwd, KEY_LOG);
	sprintf(master_key_path, "%s/%s", pwd, MASTER_KEY);
	sprintf(server_ca_path, "%s/%s", pwd, SERVER_CA);
	sprintf(server_key_path, "%s/%s", pwd, SERVER_KEY);

#ifdef DEBUG
	printf("key log路径：%s\nmaster key路径：%s\n", key_log_path, master_key_path);
#endif

	if (argv[1])
		myport = atoi(argv[1]);
	else
	{
		myport = 7838;
		argv[2] = argv[3] = NULL;
	}

	if (argv[2])
		lisnum = atoi(argv[2]);
	else
	{
		lisnum = 2;
		argv[3] = NULL;
	}

	/* SSL 库初始化 */
	SSL_library_init();
	/* 载入所有 SSL 算法 */
	OpenSSL_add_all_algorithms();
	/* 载入所有 SSL 错误消息 */
	SSL_load_error_strings();
	/* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */

	// ctx = SSL_CTX_new(SSLv23_server_method());
	// ctx = SSL_CTX_new(TLSv1_2_server_method());
	ctx = SSL_CTX_new(TLS_server_method());
	/* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
	if (ctx == NULL)
	{
		ERR_print_errors_fp(stdout);
		exit(1);
	}

// 默认情况使用tls1.2，tls1.3调试时候无法解密
#ifndef USE_TLS13
	SSL_CTX_set_max_proto_version(ctx, TLS1_2_VERSION);
#endif

	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH);

	SSL_CTX_set_ex_data(ctx, 0, key_log_path);
	SSL_CTX_set_keylog_callback(ctx, SSL_CTX_keylog_cb_func_cb);

	/* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
	if (SSL_CTX_use_certificate_file(ctx, server_ca_path, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stdout);
		exit(1);
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, server_key_path, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stdout);
		exit(1);
	}
	/* 检查用户私钥是否正确 */
	if (!SSL_CTX_check_private_key(ctx))
	{
		ERR_print_errors_fp(stdout);
		exit(1);
	}

	/* 开启一个 socket 监听 */
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(-1);
	}
	else
#ifdef DEBUG
		printf("socket created\n");
#endif

	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = PF_INET;
	my_addr.sin_port = htons(myport);
	if (argv[3])
		my_addr.sin_addr.s_addr = inet_addr(argv[3]);
	else
		my_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}
	else
#ifdef DEBUG
		printf("binded\n");
#endif

	if (listen(sockfd, lisnum) == -1)
	{
		perror("listen");
		exit(1);
	}
	else
#ifdef DEBUG
		printf("begin listen\n");
#endif

	while (1)
	{
		SSL *ssl;
		len = sizeof(struct sockaddr);
		/* 等待客户端连上来 */
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &len)) == -1)
		{
			perror("accept");
			exit(errno);
		}
		else
			printf("server: got connection from %s, port %d, socket %d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port), new_fd);

		/* 基于 ctx 产生一个新的 SSL */
		ssl = SSL_new(ctx);
		/* 将连接用户的 socket 加入到 SSL */
		SSL_set_fd(ssl, new_fd);
		/* 建立 SSL 连接 */
		if (SSL_accept(ssl) == -1)
		{
			perror("accept");
			close(new_fd);
			break;
		}

		SSL_SESSION *session = SSL_get_session(ssl);
		unsigned char master_key[SSL_MAX_MASTER_KEY_LENGTH] = { 0 };
		int master_key_len;
		master_key_len = SSL_SESSION_get_master_key(session, master_key, SSL_MAX_MASTER_KEY_LENGTH);
		if(master_key_len == 0)
		{
			printf("master_key_len: %d; master_key: \n%s\n", master_key_len, master_key);
			fprintf(stderr, "master_key_len = 0\n");
			exit(-1);
		}

		int mfd = open(master_key_path, O_CREAT | O_TRUNC | O_RDWR, 0666);
		if(mfd == -1)
		{
			fprintf(stderr, "打开master_key_path报错: %s\n", strerror(errno));
			exit(-1);
		}
		write(mfd, master_key, master_key_len);
		close(mfd);

		/* 接受客户端所传文件的文件名并在特定目录创建空文件 */
		bzero(buf, MAXBUF);
		len = SSL_read(ssl, buf, MAXBUF);
		if (len == 0)
			printf("Receive Complete !\n");
		else if (len < 0)
			printf("Failure to receive message ! Error code is %d，Error messages are '%s'\n", errno, strerror(errno));
		char file_full_path[1125];
		sprintf(file_full_path, "%s/%s", pwd, buf);
#ifdef DEBUG
		printf("读取文件名：%s\n", buf);
		printf("文件全路径：%s\n", file_full_path);
#endif
		if ((fd = open(file_full_path, O_CREAT | O_TRUNC | O_RDWR, 0666)) < 0)
		{
			perror("open");
			exit(1);
		}
		/* 接收客户端的数据并写入文件 */
		while (1)
		{
			bzero(buf, MAXBUF + 1);
			len = 0;
			len = SSL_read(ssl, buf, MAXBUF);
			if (len == 0)
			{
				printf("Receive Complete !\n");
				break;
			}
			else if (len < 0 || strlen(buf) == 0)
			{
				printf("Failure to receive message ! Error code is %d，Error messages are '%s'\n", errno, strerror(errno));
				exit(-1);
			}
			printf("收到的buf：\n%s\n", buf);
			if (write(fd, buf, len) < 0)
			{
				perror("write");
				exit(-1);
			}
		}

		/* 关闭文件 */
		close(fd);
		/* 关闭 SSL 连接 */
		SSL_shutdown(ssl);
		/* 释放 SSL */
		SSL_free(ssl);
		/* 关闭 socket */
		close(new_fd);
	}

	/* 关闭监听的 socket */
	close(sockfd);
	/* 释放 CTX */
	SSL_CTX_free(ctx);
	return 0;
}