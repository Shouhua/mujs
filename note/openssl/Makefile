all: ssl_client.c ssl_server.c
	gcc -g -Wall ssl_client.c -o ssl_client -lssl -lcrypto -DUSE_TLS13
	gcc -g -Wall ssl_server.c -o ssl_server -lssl -lcrypto
	gcc -g -Wall main.c -o main
	g++ -g -Wall bio_file.cpp -o bio_file -lssl -lcrypto
	gcc -g -Wall simple_tls.c -o simple_tls -lssl -lcrypto