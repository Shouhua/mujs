#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAXBUF 1024

void ShowCerts(SSL *ssl)
{
	X509 *cert;
	char *line;

	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL)
	{
		printf("Digital certificate information:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Certificate: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Issuer: %s\n", line);
		free(line);
		X509_free(cert);
	}
	else
		printf("No certificate information！\n");
}

int main(int argc, char **argv)
{
	int i, j=0, sockfd, len, fd, size;
	char fileName[512], sendFN[20];
	struct sockaddr_in dest;
	char buffer[MAXBUF + 1];
	SSL_CTX *ctx;
	SSL *ssl;

	if (argc != 3)
	{
		printf("Parameter format error! Correct usage is as follows：\n\t\t%s IP Port\n\tSuch as:\t%s 127.0.0.1 80\n", argv[0], argv[0]);
		exit(0);
	}

	/* SSL 库初始化 */
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	ctx = SSL_CTX_new(TLS_client_method());
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

	/* 创建一个 socket 用于 tcp 通信 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket");
		exit(errno);
	}
#ifdef DEBUG
	printf("socket created\n");
#endif

	/* 初始化服务器端（对方）的地址和端口信息 */
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(atoi(argv[2]));
	if (inet_aton(argv[1], (struct in_addr *)&dest.sin_addr.s_addr) == 0)
	{
		perror(argv[1]);
		exit(errno);
	}
#ifdef DEBUG
	printf("address created\n");
#endif

	/* 连接服务器 */
	if (connect(sockfd, (struct sockaddr *)&dest, sizeof(dest)) != 0)
	{
		perror("Connect ");
		exit(errno);
	}
#ifdef DEBUG
	printf("server connected\n\n");
#endif

	/* 基于 ctx 产生一个新的 SSL */
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sockfd);
	/* 建立 SSL 连接 */
	if (SSL_connect(ssl) == -1)
		ERR_print_errors_fp(stderr);
	else
	{
		printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
		ShowCerts(ssl);
	}

	/* 接收用户输入的文件名，并打开文件 */
	printf("\nPlease input the filename of you want to load :\n> ");
	scanf("%511s", fileName);
	if ((fd = open(fileName, O_RDONLY, 0666)) < 0)
	{
		perror("open");
		exit(1);
	}

	/* 将用户输入的文件名，去掉路径信息后，发给服务器 */
	for (i = 0; i <= strlen(fileName); i++)
	{
		if (fileName[i] == '/')
		{
			j = 0;
			continue;
		}
		else
		{
			sendFN[j] = fileName[i];
			++j;
		}
	}
	len = SSL_write(ssl, sendFN, strlen(sendFN));
	if (len < 0)
		printf("'%s'message Send failure ！Error code is %d，Error messages are '%s'\n", buffer, errno, strerror(errno));

	/* 循环发送文件内容到服务器 */
	bzero(buffer, MAXBUF + 1);
	while ((size = read(fd, buffer, 1024)))
	{
		if (size < 0)
		{
			perror("read:");
			exit(1);
		}
		else
		{
#ifdef DEBUG
			printf("内容: %s\n", buffer);
#endif
			len = SSL_write(ssl, buffer, size);
			if (len < 0)
				printf("'%s'message Send failure ！Error code is %d，Error messages are '%s'\n", buffer, errno, strerror(errno));
		}
		bzero(buffer, MAXBUF + 1);
	}
	printf("Send complete !\n");
	char str[100];
	scanf("%99s", str);
	/* 关闭连接 */
	close(fd);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	close(sockfd);
	SSL_CTX_free(ctx);
	return 0;
}