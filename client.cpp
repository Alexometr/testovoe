#include <iostream>
#include <string>
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
	std::cout << "Подключено к серверу\n";
	while (true)
	{
		std::string input;
		std::cout << ">"; //символ приглашения написать строку
		std::getline (std::cin, input); //записываем строку на экране в инпут

		if(input == "exit"){break;}
	        send(sock_descr, input.c_str(), input.length(), 0); //предварительно превратили инпут в указатель
								    //на массив байт (нужно для ф-ции send)
		char buffer[1024] = {0};
		read(sock_descr, buffer, 1024);
		std::cout << "Ответ от сервера" << buffer << "\n";

	}
	close (sock_descr);
	return 0;
}

