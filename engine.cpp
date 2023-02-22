#include <iostream>
#include <thread>
#include <mutex>

#include "io.hpp"
#include "engine.hpp"

// initialize global variable here 
OrderBook buy_orderbook = OrderBook();
OrderBook sell_orderbook = OrderBook();
//BookShelf book_shelf= BookShelf(buy_orderbook, sell_orderbook);

std::mutex buy_mutex;
std::mutex sell_mutex;
std::mutex cancel_mutex;

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

void Engine::connection_thread(ClientConnection connection)
{
	while(true) // do not exit this loop until getting a match  
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
				// acquire the buy mutex
				const std::lock_guard<std::mutex> lock (buy_mutex);
				// simply add to buy book if the sell orderbook is empty 
				if (sell_orderbook.books.empty()){
					buy_orderbook.AddtoBookAndTimeStamp(input);
				}
				else { // the matching orderbook is non-empty, then perform the matching 
					sell_orderbook.SortOrders();
					sell_orderbook.MatchOrders(input);
					auto output_time = getCurrentTimestamp();

					// loop through the books for matched orders
					for(auto it = sell_orderbook.books.rbegin(); it != sell_orderbook.books.rend(); it++){
						if((*it).matched == true){
							Output::OrderExecuted(input.order_id,(*it).order_id,(*it).execution_ID,
							(*it).price,(*it).count,(*it).time_stamp);
							(*it).matched = false; // reset matched state
						} else {
							break; // no matched order near the beginning 
						}
					}

					// check if input order has been fully filled, add to buy book if not 
					if(input.count > 0){
						buy_orderbook.AddtoBookAndTimeStamp(input);
					}
				}
			}
			case input_sell:{
				// acquire the sell mutex
				const std::lock_guard<std::mutex> lock (sell_mutex);
				// simply add to sell orderbook if the buy orderbook is empty 
				if (buy_orderbook.books.empty()){
					sell_orderbook.AddtoBookAndTimeStamp(input);
				}
				else {
					buy_orderbook.SortOrders();
					buy_orderbook.MatchOrders(input);
					auto output_time = getCurrentTimestamp();

					// loop through the books for matched orders
					for (auto it = buy_orderbook.books.rbegin(); it != buy_orderbook.books.rend(); it++){
						if((*it).matched == true){
							Output::OrderExecuted(input.order_id,(*it).order_id,(*it).execution_ID,
							(*it).price,(*it).count,(*it).time_stamp);
							(*it).matched = false; // rest matched state
						} else {
							break;
						}
					}

					// check if input order has been fully flled, add to sell book if not 
					if(input.count > 0){
						sell_orderbook.AddtoBookAndTimeStamp(input);
					}
				}
			}
			case input_cancel: {
				// SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;
				auto output_time = getCurrentTimestamp();
				bool cancel_in_buy, cancel_in_sell = false;
				{
					// acquire the sell mutex
					const std::lock_guard<std::mutex> lock (sell_mutex);
					bool cancel_in_sell = sell_orderbook.QueryAndCancelOrder(input, output_time);
				};

				{
					// acquire the buy mutex
					const std::lock_guard<std::mutex> lock (buy_mutex);
					bool cancel_in_buy = buy_orderbook.QueryAndCancelOrder(input, output_time);
				};
				
				// either one success would call the following output
				if(cancel_in_buy || cancel_in_sell){
					Output::OrderDeleted(input.order_id, true, output_time);
				}
				break;
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

