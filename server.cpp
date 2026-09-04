#include <iostream> 
#include <cstring> //для работы со строками как в си
#include <unistd.h> //для базовых вызовов в unix
#include <arpa/inet.h> //для работы с ip адресами в POSIX
#include <thread> //для потоков
#include <string>
#include <fstream> //для работы с файлами
#include <sstream> //вспомогательный инструмент для строк

const int DEFAULT_PORT = 8080;

void client_func (int client_socket) //сокет обрабатывающий клиента 
{
	const int DEFAULT_CHUNK = 1024;
	int chunk = DEFAULT_CHUNK;
	char buffer [1024] = {0};
	while (true)
	{
		for (int i = 0; i < 1024; i++)
		{buffer[i] = 0;} //чистим буфер от возможного мусора

		int valread = read(client_socket, buffer, 1024);
		if (valread <= 0) //0 - клиент закрыл сокет, -1 - ошибка сети
		{
	
			std::cout << "[Поток" << std::this_thread::get_id() << "] Клиент отключился\n";
			break;
		}
		std::string text(buffer); //тут приходит строка пути файла + размер чанка /home/alex/... (пробел) 1024
		std::string filename;		
		std::istringstream iss(text);
		iss >> filename; //тут будет считывание только до первого пробела, все что после пробела считано не будет
		iss >> chunk; //в чанк теперь запишется число чанка без пробела
		std::cout << "[Поток" << std::this_thread::get_id() << "] Запрос на скачивание файла" << filename << "\n";
		std::cout << "Размер чанка: " << chunk << " байт\n";
		std::ifstream file(filename, std::ios::binary); //читаем файл, второй аргумент открывает файл в бинарном режиме
							    //не заменяя никакие символы (важно для не текстовых файлов)

// **************************** ОТКРЫВАЕМ ФАЙЛ И ЧИТАЕМ КОЛИЧЕСТВО БАЙТ *************************
		if(!file.is_open())
		{
			std::string error = "ERROR: file not found";
			std::cout << error;
			send(client_socket, error.c_str(), error.length(), 0);
			continue;
		}
		file.seekg(0, std::ios::end); //seekg - переместить курсор в файле, std::ios::end - в конец
		long file_size = file.tellg(); //возвращает текущую позицию курсора в файле (в байтах от начала файла)
		file.seekg(0, std::ios::beg); //возвращаем курсор в начало

// ********************************** ПРОЧИТАЛИ КОЛИЧЕСТВО БАЙТ **********************************

		std::string info = "INFO: START TRANSFER" + std::to_string(file_size);
		send(client_socket, info.c_str(), info.length(),0);

// *********************************** ОТПРАВИЛИ КОЛИЧЕСТВО БАЙТ **********************************

		usleep(10000); //небольшой сон, чтобы сообщение и тело файла не пришли в одном сетевом пакете
			       //и тело файла не улетело в мусор
		char* file_buffer = new char [chunk];
		while(!file.eof()) //пока не дошли до конца файла
		{
			file.read(file_buffer, chunk); //записываем в буффер [chunk] байт данных
			int bytes_read = file.gcount(); //сколько реально записали
			if (bytes_read > 0)
			{
				send(client_socket, file_buffer, bytes_read, 0); //отправляем уже данные файла
			}
		}
		delete[] file_buffer;
		file.close();
		std::string end = "__EOF__";
		usleep(10000);
		send(client_socket, end.c_str(), end.length(), 0);
		std::cout << "Отправлен __EOF__\n";
		usleep(10000);
		std::cout << "[Поток" << std::this_thread::get_id() << "] Файл " << filename << " успешно отправлен\n";
		
// ************************* ДЛЯ ЗЕРКАЛА ******************************
		//std::cout << "[Поток" <<std::this_thread::get_id() << "]\n";
		//std::cout << "Получено байт от клиента: " << valread << "\n";
		//std::cout << "Сообщение клиента:" << buffer << "\n";
		//send(client_socket, buffer, valread, 0);
		//std::cout << "Echo send back... \n";
		//
// ************************* КОНЕЦ ЗЕРКАЛА ****************************
	}
	close(client_socket);
}

int main (int argc, char* argv[])
{
// *************************** ЗАДАЕМ ПОРТ ****************************
	int port = DEFAULT_PORT;
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
    		if (arg == "-p" && i + 1 < argc)
    		{
       			try
        		{
            			port = std::stoi(argv[i + 1]);
            			if (port <= 0 || port > 65535)
            			{
                			std::cout << "Ошибка. Порт должен быть в диапазоне 1-65535\n";
                			return 1;
            			}
           			i++;
       			 }
       			 catch (const std::exception& e)
        		 {
            			std::cout << "Ошибка: неверный формат порта\n";
            			return 1;
			 }
		}
	}
// ************************* ЗАДАЛИ ПОРТ *****************************


// ************************* ЗАДАЕМ ПАРАМЕТРЫ И ЗАПУСКАЕМ СЕРВАК *********************

	int server_descriptor = socket(AF_INET, SOCK_STREAM, 0); //создаем дескриптор сервера (дескриптор - это и есть сокет по сути) 
	//создаем сетевой сокет на ipv4; тип сокета для последовательной двусторонней передачи, соединение ТСР; 0 - нет доп флагов
	sockaddr_in address{}; //структура с семейством адресов, номером порта, айпи адресом
	//Этой строкой выделяем ровно 16 байт в ОЗУ для всего этого
	address.sin_family = AF_INET; //семейство адресов (IPv4)
	address.sin_addr.s_addr = INADDR_ANY; //айпи адрес (любой доступный)
	address.sin_port = htons(port); //порт. htons - переворачивает если надо для клиента или сервера это число
	int lenaddr = sizeof(address);
	if (bind(server_descriptor, (struct sockaddr*)&address, lenaddr) < 0) //биндим к дескриптору конкретный адрес и порт 
	{
		std::cout << "Ошибка привязки порта" << port << "\n";
		return 1;
	}
	listen(server_descriptor,10); //слушаем этот адрес и порт к которому привязались
			//максимальное количество одновременных клиентов на порту - 10
	std::cout << "Сервер запущен на порту " << port << "...\n";
	while (true)
	{
        	int new_socket = accept(server_descriptor, (struct sockaddr*)&address, (socklen_t*)&lenaddr); 
		//как только появляется клиент, accept выдает ему новый сокет, перезаписывает в address его айпи и порт
		//Когда клиентов нет, просто блокирует выполнение программы и ждет
		std::cout << "Подключился новый клиент\n";
		std::thread t(client_func, new_socket); //создали новый поток выполнения
		t.detach(); //отсоединяем наш поток от основного потока выополения
	}
	close(server_descriptor);
	return 0;
}
