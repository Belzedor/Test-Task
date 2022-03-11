#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
void sock_startup()
{
	WSADATA wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		std::cout << "Не удалось загрузить библиотеку WinSock" << std::endl;
		exit(1);
	}
}
void sock_cleanup() { WSACleanup(); }
#else
#include <condition_variable>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
void closesocket(int socket) { close(socket); }

void sock_startup() { }
void sock_cleanup() { }
#define SOCKET int
#define SOCKADDR_IN sockaddr_in 
#define SOCKADDR sockaddr 
#endif

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru");
	int data = 0;

	sock_startup();

	SOCKADDR_IN addr;
	socklen_t sizeofaddr = sizeof(addr);
	int msg_size = 0;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	while (true)
	{
		SOCKET newConnection;

		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
		if (newConnection == 0)
			std::cout << "Не удалось подключиться\n";
		else
		{
			recv(newConnection, (char*)&msg_size, sizeof(int), NULL);
			std::unique_ptr<char> msg(new char[msg_size]);
			recv(newConnection, msg.get(), msg_size, NULL);
			data = std::atoi(msg.get());
			std::cout << "Клиент подключен\n";
			closesocket(newConnection);
		}

		if (msg_size > 2 && data % 32 == 0)
			std::cout << "Данные успешно получены: " << data << '\n';
		else
			std::cout << "Полученные данные неверны " << '\n';
	}
	
	sock_cleanup();
	system("pause");
	return EXIT_SUCCESS;
}