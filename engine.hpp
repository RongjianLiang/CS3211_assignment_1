// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>

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
	uint32_t time_stamp; // probably retrieve time-stamps from a global variable protected by mutex
	char* instrument[9];
	char* type;

	// Default constructor
	RestOrder(ClientCommand command)
	{
		this->order_id = command.order_id;
		this->price = command.price;
		this->count = command.count;
		std::memcpy(instrument, command.instrument, 9);
	};

};

// dynamic array(vector) implementation of orderbook

class OrderBook{
	std::vector<RestOrder>books;
	char* inst_type[9]; // orders inside this book shall have same instrument
	char* type;			// orders inside this book shall have same type


public:
	// Default constructor
	OrderBook(){
	};

	void insertOrder(const RestOrder& order){
		books.push_back(order);
	};

	void deleteOrder(RestOrder){
		// remove the last element after queries
		books.pop_back();
	};

	RestOrder* query(RestOrder){
		// implment a sorting based on different comparator using lambdas 
		// sort by price & check validity
		std::sort(this->books.begin(), this->books.end(),[](const RestOrder& a, const RestOrder& b)
		{
			return a.price > b.price;
		});
		// sort by time added & check validity 
		std::sort(this->books.begin(), this->books.end(),[](const RestOrder& a, const RestOrder& b)
		{
			return a.time_stamp < b.time_stamp;
		});


	}
	;

};

// a timstamping scheme should be implemented here?
inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
