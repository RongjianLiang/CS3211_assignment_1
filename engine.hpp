// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>
#include <iostream>
#include <cstring>
#include <vector>

#include "io.hpp"

struct Engine
{
public:
	void accept(ClientConnection conn);

private:
	void connection_thread(ClientConnection conn);
};

// define a data structure for order book and order book entries
// create sell orderbooks and buys orderbooks
class RestOrder {
public:
	uint32_t order_id;
	uint32_t price;
	uint32_t count;
	char* instrument[9];
	char* type;

	// Default constructor
	RestOrder(ClientCommand command)
	{
		order_id = command.order_id;
		price = command.price;
		count = command.count;
		std::memcpy(instrument, command.instrument, 9);
	};

};

// dynamic array(vector) implementation of orderbook

class OrderBook{
	std::vector<RestOrder>books;
	char* inst_type[9]; // orders inside this book shall have same instrument


public:
	// Default constructor
	OrderBook(){


	};

	void insertOrder(RestOrder){

	};

	void deleteOrder(RestOrder){

	};

	RestOrder* query(RestOrder){
		
	}
	;

};

// a timstamping scheme should be implemented here?
inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
