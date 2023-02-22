// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <string>

#include "OrderBook.cpp"
#include "io.hpp"

// a timstamping scheme should be implemented here?
inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

struct Engine
{
public:
	void accept(ClientConnection conn);

private:
    std::mutex buy_order_book_mutex;
    std::mutex sell_order_book_mutex;
    OrderBook buy_orderbook = OrderBook();
    OrderBook sell_orderbook = OrderBook();
    void connection_thread(ClientConnection conn);
};

// a vector storing pointers to different OrderBooks to achieve instrument-level concurrency 
class BookShelf{
	std::vector<OrderBook>shelf;

	public:
	BookShelf(OrderBook& bookA, OrderBook& bookB){
		this->shelf.push_back(bookA);
		this->shelf.push_back(bookB);
	};

	void addToShelf (OrderBook orderbook){
		shelf.push_back(orderbook);
	}

	// OrderBook* queryShelf(ClientCommand input){
	// 	for(auto it = this->shelf.begin(); it != this->shelf.end(); it++){
	// 		if(*(*it).inst_type == input.instrument){
	// 		}
	// 	}
	// }
};
#endif
