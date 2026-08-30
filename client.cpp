#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <fstream>

int main ()
{
	int sock_descr = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serv_addr{};
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
	connect(sock_descr, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	std::cout << "Подключено к серверу\n";
	char buffer[1024] = {0};	
	while (true)
	{
		std::string input;
		std::cout << ">"; //символ приглашения написать строку
		std::getline (std::cin, input); //записываем строку на экране в инпут

		if(input == "exit"){break;}
	        send(sock_descr, input.c_str(), input.length(), 0); //предварительно превратили инпут в указатель
								    //на массив байт (нужно для ф-ции send)	
		for (int i = 0; i < 1024; i++){buffer[i] = 0;}
		int valread = read(sock_descr, buffer, 1024);
		if (valread <= 0)
		{
			std::cout << "Сервер разорвал соединение...\n";
			break;
		}
		std::string response(buffer);
		if (response == "INFO: START TRANSFER")
		{
			std::string filename = input.substr(9);
			std::cout << "Файл найден. Скачивание...\n";
			std::ofstream file("downloaded_" + filename, std::ios::binary);
			while(true)
			{
				for(int i = 0; i < 1024; i++) {buffer[i] = 0;}
				int bytes_receive = read(sock_descr, buffer,1024);
				if (bytes_receive <=0){break;}
				file.write(buffer, bytes_receive);
			}
			file.close();
			std::cout << "Файл сохранен как: downloaded_" << filename << "\n";
			break;
		}
		else if (response == "ERROR: file not found")
		{
			std::cout << "Файл не найден на сервере\n";
		}
		else
		{
			std::cout << "Ответ от сервера" << buffer << "\n";
		}

	}
	close (sock_descr);
	return 0;
}

