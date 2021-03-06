#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <kapok/Kapok.hpp>
#include "client_proxy.hpp"
#include "base64.hpp"
#include "../common.h"

struct person
{
	int age;
	std::string name;

	META(age, name);
};

template<typename T>
void handle_result(const char* result)
{
	DeSerializer dr;
	dr.Parse(result);
	Document& doc = dr.GetDocument();
	doc.Parse(result);
	if (doc[CODE].GetInt() == result_code::OK)
	{
		response_msg<T> response = {};
		dr.Deserialize(response);
		std::cout << response.result << std::endl;
	}
	else
	{
		//maybe exception, output the exception message.
		std::cout << doc[RESULT].GetString() << std::endl;
	}
}

void test_async_client()
{
	try
	{
		boost::asio::io_service io_service;
		client_proxy client(io_service);
		client.async_connect("baidu.com", "9000", 5*1000, [&client] (boost::system::error_code& ec)
		{
			if (ec)
			{
				std::cout << "connect error." << std::endl;
				return;
			}

			client.async_call("add", [&client](boost::system::error_code ec, std::string result)
			{
				if (ec)
				{
					std::cout << "call error." << std::endl;
					return;
				}

				handle_result<int>(result.c_str());
			},1, 2);

		});

		io_service.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

void test_spawn_client()
{
	try
	{
		boost::asio::io_service io_service;
		boost::asio::spawn(io_service, [&io_service] (boost::asio::yield_context yield)
		{
			client_proxy client(io_service);
			client.async_connect("127.0.0.1", "9000", yield);

			std::string result = client.async_call("add", yield, 1,2);
			handle_result<int>(result.c_str());
		});
		io_service.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

void test_translate()
{
	try
	{
		boost::asio::io_service io_service;
		client_proxy client(io_service);
		client.connect("127.0.0.1", "9000");
		
		std::string result = client.call("translate", "test");
		handle_result<std::string>(result.c_str());

		io_service.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

int main()
{
	log::get().init("rest_rpc_client.lg");
	test_translate();

	return 0;
}