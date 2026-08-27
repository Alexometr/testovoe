#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main ()
{
	int sock_descr = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serv_addr{};
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	connect(sock_descr, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	char* message = "Hello world!";
	int byte_ = send(sock_descr, message, strlen(message), 0);
	std::cout << "Отправлено: " << byte_ <<"байт\n";
	std::cout << "Отправленное сообщение: " << message << "\n";
	char buffer [1024] = {0};
	int responce = read(sock_descr, buffer, 1024);
	std::cout << "Принято от сервера: " << responce << "байт\n";
	std::cout << "Ответ от сервера: " << buffer << "\n";
	close(sock_descr);
	return 0;
}

