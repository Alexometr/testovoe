#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <fstream> //для работы с файлами
#include <cstring>

int main (int argc, char* argv[])
{
	const int DEFAULT_CHUNK = 1024;
	const int DEFAULT_PORT = 8080;
	int chunk = DEFAULT_CHUNK;
	int port = DEFAULT_PORT;
	if(argc < 5 || argc > 9)
	{
		std::cout << "Ошибка, неверный формат команды.\n";
		std::cout << "Пример: client_app [-chunk 2048] [-p 9090] -src 127.0.0.1:/tmp/file_src -dst /tmp/file_dst\n";
		return 1;
	}
	
	std::string source_param = "";
	std::string destination_path = "";
	for (int i = 1; i < argc; i++) //защита от перестановок аргументов местами
	{
		std::string arg = argv[i];
		if(arg == "-src" && i+1 < argc){source_param = argv[i+1]; i++;}
		else if(arg == "-dst" && i+1 < argc){destination_path = argv[i+1]; i++;}

// ********************* УСТАНАВЛИВАЕМ РАЗМЕР ЧАНКА **************************
		else if(arg == "-chunk" && i+1 < argc)
		{
			try
			{
				chunk = std::stoi(argv[i+1]);
				if(chunk <= 0)
				{
					std::cout << "Ошибка. Размер чанка должен натуральным числом\n";
					return 1;
				}
				if(chunk > 1024*1024)
				{
					std::cout << "Размер чанка слишком большой. Установлен размер 1 Мб\n";
					chunk = 1024*1024;
				}
				i++;
			}
			catch (const std::exception& e)
       			{
            			std::cout << "Ошибка: неверный формат размера чанка\n";
            			return 1;
        		}
		}

// ***************************** УСТАНОВИЛИ РАЗМЕР ЧАНКА ***************************

		else if(arg == "-p" && i+1 < argc)
		{
			try
			{
				port = std::stoi(argv[i+1]);
				if(port < 0 || port > 65535)
				{
					std::cout << "Ошибка. Порт должен быть в диапазоне 1 - 65535\n";
					return 1;
				}
				i++;
			}
			catch (const std::exception& e)
			{
				std::cout << "Ошибка. Неверный формат порта\n";
				return 1;
			}
		}
	}
// ********************************* УСТАНОВИЛИ ПОРТ **********************************

	if(source_param.empty() || destination_path.empty())
	{
		std::cout << "Ошибка пропущены параметры -src или -dst\n";
		return 1;
	}

// ************************ ИЩЕМ ДВОЕТОЧИЕ МЕЖДУ АЙПИ И ПУТЕМ *************************
	size_t position = source_param.find(":"); //ф-ции .size(), .length() и методы .(r)find() всегда возвращают size_t
						  //возвращает индекс символа
	if (position == std::string::npos) //если find() не нашел символ (фактически там -1)
	{
		std::cout << "Ошибка. Пропущено двоеточие между IP и путем\n";
		return 1;
	}

// ****************** ПАРСИМ АЙПИ И ПУТЬ + ПОДКЛЮЧАЕМСЯ К СЕРВЕРУ *********************

	std::string ip_addr = source_param.substr(0, position); //IP адрес сервера. substr (позиция откуда берем подстроку,
								//сколько символов берем). Т.е. у нас 127:..., то
								// : - индекс 3, 1 - индекс 0, 127 - как раз 3 символа
	std::string server_file_path = source_param.substr(position + 1); //путь к файлу на сервере

	int sock_descr = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serv_addr{};
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip_addr.c_str(), &serv_addr.sin_addr);
	std::cout << "Подключение к серверу " << ip_addr << "...\n";
	std::cout << "Порт:" << port << "\n";
	if(connect(sock_descr, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cout << "Ошибка подключения к серверу.\n";
		close(sock_descr);
		return 1;
	}
	else
	{
		std::cout << "Подключено к серверу\n";
	}

// *************************** ПОДКЛЮЧИЛИСЬ К СЕРВЕРУ *******************************

	std::string command = server_file_path + " " + std::to_string(chunk);
	char* buffer = new char[chunk];
	memset(buffer, 0, chunk); //заполняет заданный участок памяти определенным байтом 
	send(sock_descr, command.c_str(), command.length(), 0); //первая отправка: имя нужного файла + чанк

	int valread = read(sock_descr, buffer, 1024); //первое получение: уведомл о начале отправки + размер файла
	if (valread <= 0)
	{
		std::cout << "Сервер разорвал соединение...\n";
		close(sock_descr);
		return 1;
	}
	std::string response(buffer);
	if (response.rfind ("INFO: START TRANSFER", 0) == 0)
	{
		long file_size = 0;
		std::string rest = response.substr(20); //размер файла
		if(!rest.empty())
		{
			try
			{
				file_size = std::stoi(rest);
			}
			catch(...)
			{
				file_size = 0;
			}
		}
		std::cout << "Файл найден. Размер: "<< file_size << " байт\n";
		std::cout << "Скачивание...\n";
		char* file_buffer = new char[chunk];
		std::ofstream file(destination_path, std::ios::binary); //запись в файл по пути назначения
									//+ открытие в бинарном формате
		long total_bytes = 0;
		while(true)
		{
			for(int i = 0; i < chunk; i++) {buffer[i] = 0;}

// ************* ПРОИСХОДИТ СКАЧИВАНИЕ ФАЙЛА И НЕМЕДЛЕННАЯ ЗАПИСЬ В ЛОКАЛЬНЫЙ ФАЙЛ **************

			int bytes_receive = read(sock_descr, buffer, chunk); //второе чтение. Чтение порции данных из файла
			if (bytes_receive <= 0){break;}
			std::string data(buffer, bytes_receive); //создали строку и заполнили bytes_receive байт данными из буфера
			if (bytes_receive >= 7 && data.substr(bytes_receive - 7) == "__EOF__")
    			{
				int write_bytes = bytes_receive - 7; //убираем флаг конца файла
        			if (write_bytes > 0)
        			{
            				file.write(buffer, write_bytes);
            				total_bytes += write_bytes;
        			}
        			break;
    			}
			file.write(buffer, bytes_receive);
			total_bytes += bytes_receive;
			if (file_size > 0)
			{
				int percent = (int)((total_bytes * 100) / file_size);
                		std::cout << "\rПередано " << total_bytes << " байт/" << file_size << " байт (" << percent << "%)" <<std::flush;
			}
			else 
			{
                		std::cout << "\rПередано " << total_bytes << " байт" <<std::flush;
			}

		}
		std::cout << std::endl;
		file.close();
		std::cout << "Скачивание успешно завершено\n";
	}
// *********************** СКАЧАЛИ, ЗАПИСАЛИ И ЗАКРЫЛИ ЛОКАЛЬНЫЙ ФАЙЛ *********************

	else if (response == "ERROR: file not found")
	{
		std::cout << "Файл не найден на сервере\n";
	}
	else
	{
		std::cout << "Ответ от сервера" << buffer << "\n";
	}
	close (sock_descr);
	return 0;
}

