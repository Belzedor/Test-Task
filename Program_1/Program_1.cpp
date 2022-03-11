#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>


#ifdef _WIN32
#include <WinSock2.h>
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

class String_proc
{
public:
	void input()
	{
		while (true)
		{
			try
			{
				std::cout << "-------------------------" << '\n';
				std::cout << "Введите данные:" << '\n';
				std::cin >> input_data;
				std::cout << input_data.size() << '\n';
				if (input_data.size() > 64 || input_data.size() < 2)
					throw std::runtime_error("Неверный размер строки");
				std::sort(input_data.begin(), input_data.end(), [](char a, char b) {return a > b;});
				std::unique_lock<std::mutex> lock(mutex);
				for (auto c : input_data) 
				{
					if (c < '0' || c > '9')
						throw std::runtime_error("Введены неверные данные");
					(c - '0') % 2 ? buffer += c : buffer += "KB";
				}
				lock.unlock();
				cvar.notify_one();
			}
			catch (std::exception const& err)
			{
				std::cerr << "Произошла ошибка: " << err.what() << std::endl;
				check_thread = true;
				cvar.notify_one();
				return;
			}
		}
	}

	void sum_transfer()
	{
		SOCKADDR_IN addr;
		int sizeofaddr = sizeof(addr);
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(1111);
		addr.sin_family = AF_INET;
		
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(mutex);
				cvar.wait(lock);
				if (check_thread) return;
				transfer_data = buffer;
				buffer = "";
			}

			sum = 0;
			std::cout << "Полученные данные: " << transfer_data << '\n';
			for (char elem : transfer_data)
			{
				if (elem > '0' && elem <= '9')
					sum += elem - '0';
			}
			
			SOCKET Connection = socket(AF_INET, SOCK_STREAM, NULL);

			if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
				std::cout << "Сервер недоступен. Данные не отправлены\n";
			else
			{
				std::cout << "Данные успешно отправлены\n";
				std::string msg = std::to_string(sum);
				int msg_size = msg.size();
				send(Connection, (char*)&msg_size, sizeof(int), NULL);
				send(Connection, msg.c_str(), msg_size, NULL);
			}
		}
	}
private:
	std::string input_data;
	std::string buffer;
	std::string transfer_data;
	int sum = 0;
	std::mutex mutex;
	std::condition_variable cvar;
	bool check_thread = false;
};

int main(int argc, char* argv[])
{
	try
	{
		sock_startup();
		setlocale(LC_ALL, "ru");

		String_proc proc;
		std::thread t1([&]() {proc.input();});
		std::thread t2([&]() {proc.sum_transfer();});
		t1.join();
		t2.join();

		sock_cleanup();
		system("pause");
	}
	catch (std::exception const& err)
	{
		std::cerr << "Произошла ошибка: " << err.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}