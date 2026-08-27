#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main ()
{
	int server_descriptor = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8080);
	int lenaddr = sizeof(address);
	bind(server_descriptor, (struct sockaddr*)&address, lenaddr);
	listen(server_descriptor,3);
	std::cout << "Сервер запущен на порту 8080...\n";
	int new_socket = accept(server_descriptor, (struct sockaddr*)&address, (socklen_t*)&lenaddr);
	char buffer [1024] = {0};
	int valread = read(new_socket, buffer, 1024);
	std::cout << "Получено байт от клиента: " << valread << "\n";
	std::cout << "Сообщение от клиента: " << buffer << "\n";
	send (new_socket, buffer, valread, 0);
	std::cout << "Echo send back...\n";
	close(new_socket);
	close(server_descriptor);
	return 0;

}
