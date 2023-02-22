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
#include <unordered_map>

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
	void connection_thread(ClientConnection conn);
    std::mutex get_instrument_book_mutex;
};

// define a data structure for order book and order book entries
// create sell orderbooks and buys orderbooks
class RestOrder {
public:
	uint32_t order_id;
	uint32_t price;
	uint32_t count;
	uint32_t execution_ID;
	std::chrono::microseconds::rep time_stamp; // probably retrieve time-stamps from a global variable protected by mutex
	char* instrument[9];
	std::string type;
	bool matched;

	// Default constructor
	RestOrder(ClientCommand command)
	{
		this->order_id = command.order_id;
		this->price = command.price;
		this->count = command.count;
		this->matched = false;
		this->execution_ID = 0;
		// uint32_t time_stamp = 
		std::memcpy(instrument, command.instrument, 9);
	};
};

// dynamic array(vector) implementation of orderbook
class OrderBook{
	public:
		std::vector<RestOrder>books;
		char* inst_type[9]; // orders inside this book shall have same instrument
		std::string type;			// orders inside this book shall have same type

public:
	// Default constructor
	OrderBook(){
		//this->type = type;
	};

	void setInst (char* inst){
		std::memcpy(&(this->type), inst, 9);
	}

	// add order from client command and time-stamping
	void AddtoBookAndTimeStamp(ClientCommand& input){
		// this section should be protected 
		RestOrder order = RestOrder(input); // the RestOrder constructor should time-stamp
		order.time_stamp = getCurrentTimestamp();
		books.push_back(order); 
	};

	// sort the vector contains orders
	void SortOrders (){ 
		// sort by price & check validity
		std::sort(this->books.begin(), this->books.end(),[this](const RestOrder& a, const RestOrder& b)
		{
			if(a.price != b.price && (((this->type).compare("S"))== 0)) {
				// orderbook is all selling orders, sort the price in descending order
				return a.price < b.price;
			}
			if(a.price != b.price && (((this->type).compare("B"))== 0)) {
				// orderbook is all buying orders, sort the price in ascending order
				return a.price > b.price;
			}
			else { // if same price, sort the time stamp in descending order
				return a.time_stamp < b.time_stamp;
			}
		});
	}
	
	// loop reversely and try to match, with validity check: B price > S price
	// matchable resting orders would have their execution ID incremented 
	// and have their "matched" member set to true for this match
	// implement checking for inst for initial testing 
	// erase fully executed orders from orderbook at the end  
	void MatchOrders (ClientCommand& input){
		bool thisIsBuy = (this->type.compare("B") == 0);
		bool thisIsSell = (this->type.compare("S") == 0);
		for(auto it = this->books.rbegin(); it != this->books.rend(); it++){
			// B price > S price
			if((thisIsBuy && (*it).price >= input.price && *(*it).instrument == input.instrument) || 
			   (thisIsSell && (*it).price <= input.price && *(*it).instrument == input.instrument)){
				// new order fully filled, with resting order fully or partially filled
				if(input.count <= (*it).count){
				(*it).execution_ID ++;
				(*it).matched = true;
				(*it).count -= input.count;
				input.count = 0;
				}
				// new order partially or fully filed, with resting order fully filled 
				if(input.count >= (*it).count){
				(*it).execution_ID ++;
				(*it).matched = true;
				(*it).count = 0;
				input.count -= (*it).count;
				}
			} 
			else {
				continue;
			}			
		}
		// erase fully executed orders from orderbook at the end 
		// auto erased = std::erase_if(this->books, [](RestOrder order){ return (order.count == 0);});
	}
	
	// return true if successfully cancelled and delete the order by std::vector.erase
	// false if rejected, either deleted or not-exist
	// attempt to cancel deleted order would simply return false, as no such match exists. 
	// the timestamp is determined in engine.cpp 
	bool QueryAndCancelOrder(ClientCommand input, uint32_t time_stamp){
		bool result = false;
		int index = 0;

		for(auto it = this->books.begin(); it != this->books.end(); it++){
			if((*it).order_id == input.order_id){
				// compare the timestamp and check if the order is fully filled 
				result = (((*it).time_stamp < time_stamp) && (*it).count > 0); 
			}
			index ++;
		}
		// purge the order
		if (result == true){
			this->books.erase(books.begin() + index);
		}
		return result;
	}
};

#endif
