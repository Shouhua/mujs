// gcc bio_file.c -o bio_file -lssl -lcrypto
#include <stdio.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>

#define MAX_BUFFER_SIZE 512

int main(int argc, char *argv[])
{
	printf("OpenSSL version: %s\n", OpenSSL_version(SSLEAY_VERSION));

	BIO *bio_out = NULL;
	BIO *bio_in = NULL;
	BIO *bio_b64 = NULL;
	int inByte, outByte;
	char buffer[MAX_BUFFER_SIZE];
	memset(buffer, '\0', MAX_BUFFER_SIZE);

	if (argc != 3)
	{
		printf("Usage: bio_file <file-read> <file-write>");
		return 1;
	}

	bio_in = BIO_new_file(argv[1], "r");
	bio_out = BIO_new_file(argv[2], "wb");
	while ((inByte = BIO_read(bio_in, buffer, MAX_BUFFER_SIZE)) > 0)
	{
		outByte = BIO_write(bio_b64, buffer, inByte);
		if (inByte != outByte)
		{
			printf("In Bytes: %d Out Bytes: %d\n", inByte, outByte);
			break;
		}
	}

	BIO_free(bio_in);
	BIO_free(bio_out);
	return 0;
}