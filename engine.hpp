// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>

#include "io.hpp"

struct Engine
{
public:
	void accept(ClientConnection conn);

private:
	void connection_thread(ClientConnection conn);
};

// define a data structure for order book and order book entries
class Order {
public:
	uint32_t order_id;
	uint32_t price;
	uint32_t count;
	char instrument[9];
	Order* next;

	// Default constructor
	Order()
	{
		order_id = 0;
		price = 0;
		count = 0;
		//instrument = " ";
		next = NULL;
	}

	// Parameterised constructor
	Order()
	{
		//this->data = data;
		this->next = NULL;
	}
};

// single-linked list implementation of orderbook

class OrderBook{
	Order* head;

public:
	// Default constructor
	OrderBook(){ head = NULL; }

	void insertOrder(Order);

	void deleteOrder(Order);

};

// a timstamping scheme should be implemented here?
inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
