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
	bool matched;

	// Default constructor
	RestOrder(ClientCommand command)
	{
		this->order_id = command.order_id;
		this->price = command.price;
		this->count = command.count;
		this->matched = false;
		// uint32_t time_stamp = 
		std::memcpy(instrument, command.instrument, 9);
	};
	// Move constructor 

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

	// add order from client command and time-stamping
	void AddtoBookAndTimeStamp(ClientCommand& input){
		// this section should be protected 
		RestOrder order = RestOrder(input); // the RestOrder constructor should time-stamp
		books.push_back(order); 
	};

	// return the reference to last order in the book, and delete the order if no units left
	// check the match status of all orders in the book 
	RestOrder* ReturnAndDelete(RestOrder& order){
		for(auto it = this->books.rbegin(); it != this->books.rend(); it++){
			if((*it).matched == true){
				if ((*it).count > 0){
					return &(*it);
				}
	
				if ((*it).count == 0){
					RestOrder returnOrder = (*it);
					books.pop_back();
					return &returnOrder;
				}
			}
			else {
				break; // there couldnot be any matched order near the beginning 
			}
		}
	};
	
	// sort the vector and match 
	void SortAndMatch (ClientCommand& input){ 
		// if the vector is empty, just add and return 
		if(this->books.empty()){
			AddtoBookAndTimeStamp(input);
			return;
		}
		// sort by price & check validity
		std::sort(this->books.begin(), this->books.end(),[this](const RestOrder& a, const RestOrder& b)
		{
			if(a.price != b.price && this->type == "S"){
				// orderbook is all selling orders, sort the price in descending order
				return a.price < b.price;
			}
			if(a.price != b.price && this->type == "B"){
				// orderbook is all buying orders, sort the price in ascending order
				return a.price > b.price;
			}
			else { // if same price, sort the time stamp in descending order
				return a.time_stamp < b.time_stamp;
			}
		});
		// loop reversely and try to match, with validity check: B price > S price
		for(auto it = this->books.rbegin(); it != this->books.rend(); it++){
			// B price > S price
			if((this->type == "B" && (*it).price >= input.price) || 
			   (this->type == "S" && (*it).price <= input.price)){
				// new order fully filled, with resting order fully or partially filled
				if(input.count <= (*it).count){
				(*it).matched = true;
				(*it).count -= input.count;
				input.count = 0;
				}
				// new order partially or fully filed, with resting order fully filled 
				if(input.count >= (*it).count){
				(*it).matched = true;
				(*it).count = 0;
				input.count -= (*it).count;
				}
			} 
			else {
				continue;
			}			
		}
		// if the new order is still not filled, add to books
		if(input.count > 0 ){
			AddtoBookAndTimeStamp(input);
		}
	}
};

// a vector storing pointers to different OrderBooks
class BookShelf{
	std::vector<OrderBook>shelf;

	public:
	BookShelf(){
	};

	void addToShelf (OrderBook orderbook){
		shelf.push_back(orderbook);
	}
	
};

// a timstamping scheme should be implemented here?
inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

#endif
