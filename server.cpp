#include <iostream> 
#include <cstring> //для работы со строками как в си
#include <unistd.h> //для базовых вызовов в unix
#include <arpa/inet.h> //для работы с ip адресами в POSIX
#include <thread> //для потоков

void client_func (int client_socket) //сокет обрабатывающий клиента 
{
	char buffer [1024] = {0};
	while (true)
	{
		for (int i = 0; i < 1024; i++)
		{buffer[i] = 0;} //чистим буфер от возможного мусора

		int valread = read(client_socket, buffer, 1024);
		if (valread <=0) //0 - клиент закрыл сокет, -1 - ошибка сети
		{
	
			std::cout << "[Поток" << std::this_thread::get_id() << "] Клиент отключился\n";
			break;
		}
		std::cout << "[Поток" <<std::this_thread::get_id() << "]\n";
		std::cout << "Получено байт от клиента: " << valread << "\n";
		std::cout << "Сообщение клиента:" << buffer << "\n";
		send(client_socket, buffer, valread, 0);
		std::cout << "Echo send back... \n";
		
	}
	close(client_socket);
}

int main ()
{
	int server_descriptor = socket(AF_INET, SOCK_STREAM, 0); //создаем дескриптор сервера (дескриптор - это и есть сокет по сути) 
	//создаем сетевой сокет на ipv4; тип сокета для последовательной двусторонней передачи, соединение ТСР; 0 - нет доп флагов
	sockaddr_in address{}; //структура с семейством адресов, номером порта, айпи адресом
	//Этой строкой выделяем ровно 16 байт в ОЗУ для всего этого
	address.sin_family = AF_INET; //семейство адресов (IPv4)
	address.sin_addr.s_addr = INADDR_ANY; //айпи адрес (любой доступный)
	address.sin_port = htons(8080); //порт. htons - переворачивает если надо для клиента или сервера это число
	int lenaddr = sizeof(address);
	bind(server_descriptor, (struct sockaddr*)&address, lenaddr); //биндим к дескриптору конкретный адрес и порт 
	listen(server_descriptor,10); //слушаем этот адрес и порт к которому привязались
			//максимальное количество одновременных клиентов на порту - 10
	std::cout << "Сервер запущен на порту 8080...\n";
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
