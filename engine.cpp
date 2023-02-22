#include <iostream>
#include <thread>
#include <mutex>

#include "io.hpp"
#include "engine.hpp"

// initialize global variable here 
OrderBook buy_orderbook = OrderBook();
OrderBook sell_orderbook = OrderBook();
//BookShelf book_shelf= BookShelf(buy_orderbook, sell_orderbook);

std::mutex buy_order_book_mutex;
std::mutex sell_order_book_mutex;

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::connection_thread(ClientConnection connection)
{
	while(true) // do not exit this loop until getting a match or get the job done  
	{
		ClientCommand input {};// empty input?
		switch(connection.readInput(input))
		{
			case ReadResult::Error: SyncCerr {} << "Error reading input" << std::endl;
			case ReadResult::EndOfFile: return;
			case ReadResult::Success: break;
		}

		// Functions for printing output actions in the prescribed format are
		// provided in the Output class:
		switch(input.type)
		{
			case input_buy:{
				// simply add to buy book if the sell orderbook is empty, need to acquire buy orderbook mutex here 
				if (sell_orderbook.books.empty()){
					// std::cout << "empty sell orderbook" << std::endl;
					uint32_t time = getCurrentTimestamp();
					const std::lock_guard<std::mutex> lock (buy_order_book_mutex);
					buy_orderbook.AddtoBookwithTimeStamp(input, time);
					Output::OrderAdded(input.order_id, input.instrument,input.price,input.count,false,time);
				}
				else { // the matching orderbook is non-empty, then perform the matching
					// acquire the sell mutex
					const std::lock_guard<std::mutex> lock (sell_order_book_mutex); 
					sell_orderbook.SortOrders();
					sell_orderbook.MatchOrders(input);
					auto output_time = getCurrentTimestamp();

					// loop through the books for matched orders
					for(auto it = sell_orderbook.books.rbegin(); it != sell_orderbook.books.rend(); it++){
						if((*it).matched == true){
							Output::OrderExecuted(input.order_id,(*it).order_id,(*it).execution_ID,
							(*it).price,(*it).count,output_time);
							(*it).matched = false; // reset matched state
							(*it).time_stamp = output_time; // update the timestamp after execution, for cancelling orders 
						} else {
							break; // no more matched order near the beginning 
						}
					}

					// check if input order has been fully filled, add to buy book if not, need to acqurie buy mutex here 
					if(input.count > 0){
						uint32_t time = getCurrentTimestamp();
						const std::lock_guard<std::mutex> lock (buy_order_book_mutex);
						buy_orderbook.AddtoBookwithTimeStamp(input, time);
						Output::OrderAdded(input.order_id, input.instrument,input.price,input.count,true,time);
					}
				}
			}
			case input_sell:{
				// simply add to sell orderbook if the buy orderbook is empty, need to acquire sell mutex
				if (buy_orderbook.books.empty()){
					const std::lock_guard<std::mutex> lock (sell_order_book_mutex);
					uint32_t time = getCurrentTimestamp();
					sell_orderbook.AddtoBookwithTimeStamp(input,time);
				}
				else {
					// acquire the buy mutex
					const std::lock_guard<std::mutex> lock (buy_order_book_mutex);
					buy_orderbook.SortOrders();
					buy_orderbook.MatchOrders(input);
					auto output_time = getCurrentTimestamp();

					// loop through the books for matched orders
					for (auto it = buy_orderbook.books.rbegin(); it != buy_orderbook.books.rend(); it++){
						if((*it).matched == true){
							Output::OrderExecuted(input.order_id,(*it).order_id,(*it).execution_ID,
							(*it).price,(*it).count,output_time);
							(*it).matched = false; // rest matched state
							(*it).time_stamp = output_time; // update the timestamps, for cancelling orders 
						} else {
							break;
						}
					}

					// check if input order has been fully flled, add to sell book if not, need to acquire sell mutex here
					if(input.count > 0){
						const std::lock_guard<std::mutex> lock (sell_order_book_mutex);
						uint32_t time = getCurrentTimestamp();
						sell_orderbook.AddtoBookwithTimeStamp(input, time);
					}
				}
			}
			case input_cancel: {
				// SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;
				auto output_time = getCurrentTimestamp(); // shall we protect the time variable as well?
				bool cancel_in_buy = false;
				bool cancel_in_sell = false;
				{
					// acquire the sell mutex
					const std::lock_guard<std::mutex> lock (sell_order_book_mutex);
					bool cancel_in_sell = sell_orderbook.QueryAndCancelOrder(input, output_time);
					std::cout <<"is it cancelled in sell? " << cancel_in_sell <<std::endl;
				};

				{
					// acquire the buy mutex
					const std::lock_guard<std::mutex> lock (buy_order_book_mutex);
					bool cancel_in_buy = buy_orderbook.QueryAndCancelOrder(input, output_time);
					std::cout <<"is it cancelled in buy? " << cancel_in_sell <<std::endl;
				};
				
				// either one success would call the following output
				Output::OrderDeleted(input.order_id, (cancel_in_buy || cancel_in_sell), output_time);
			}

			default: {
				SyncCerr {}
				    << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x " << input.count << " @ "
				    << input.price << " ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				auto output_time = getCurrentTimestamp();
				Output::OrderAdded(input.order_id, input.instrument, input.price, input.count, input.type == input_sell,
				    output_time);
				break;
			}
		}

		// Additionally:

		// Remember to take timestamp at the appropriate time, or compute
		// an appropriate timestamp!
		intmax_t output_time = getCurrentTimestamp();

		// Check the parameter names in `io.hpp`.
		Output::OrderExecuted(123, 124, 1, 2000, 10, output_time);
	}
}

