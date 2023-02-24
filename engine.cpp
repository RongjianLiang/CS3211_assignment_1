#include <iostream>
#include <thread>
#include <mutex>

#include "io.hpp"
#include "engine.hpp"

void Engine::accept(ClientConnection connection)
{
	auto thread = std::thread(&Engine::connection_thread, this, std::move(connection));
	thread.detach();
}

std::string Engine::getInstrumentForOrderId(uint32_t order_id) {
	return orderIdsToInstrumentsMap.at(order_id);
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
			case input_buy:
            {
				// get the instrument - for cancel, this is an empty instrumentName
				std::shared_ptr<Instrument> instrumentPtr = bookShelf.getInstrumentBooksIfExistOrElseAddAndGet(std::string(input.instrument));
                OrderBook& sell_orderbook = instrumentPtr->sellBook;
				
                // simply add to buy book if the sell orderbook is empty, need to acquire buy orderbook mutex here 
				if (sell_orderbook.books.empty()){
					chrono_reps time = getCurrentTimestamp();
					const std::lock_guard<std::mutex> lock {instrumentPtr->instrument_buy_book_mutex};
					(instrumentPtr->buyBook).AddtoBookwithTimeStamp(input, time, orderIdsToInstrumentsMap);
					Output::OrderAdded(input.order_id, input.instrument,input.price,input.count,false,time);
				}
				else { // the matching orderbook is non-empty, then perform the matching
					// acquire the sell mutex
					const std::lock_guard<std::mutex> lock{instrumentPtr->instrument_sell_book_mutex};
					sell_orderbook.SortOrders();
					sell_orderbook.MatchOrders(input, orderIdsToInstrumentsMap);
					chrono_reps output_time = getCurrentTimestamp();

					// loop through the books for matched orders
					for(auto it = sell_orderbook.books.rbegin(); it != sell_orderbook.books.rend(); it++){
						if((*it).matched == true){
							Output::OrderExecuted(input.order_id,(*it).order_id,(*it).execution_ID,
							(*it).price,(*it).count,output_time);
							(*it).matched = false; // reset matched state
							(*it).time_stamp = output_time; // update the timestamp after execution, for cancelling orders
		                    // erase fully executed orders from orderbook at the end 
		                    // auto erased = std::erase_if(this->books, [](RestOrder order){ return (order.count == 0);});
                            if ((*it).count == 0) {
                                sell_orderbook.erase(it); 
                                orderIdsToInstrumentsMap.erase((*it).order_id);
                            }
						}/* else {
							break; // no more matched order near the beginning 
						}*/
					}
                    
                    // TOOD partial fulfillment

					// check if input order has been fully filled, add to buy book if not, need to acqurie buy mutex here 
					// if(input.count > 0){
					// 	std::cout << "should only execute this code after matching with remaining units..."<<std::endl;
					// 	chrono_reps time = getCurrentTimestamp();
					// 	const std::lock_guard<std::mutex> lock (buy_order_book_mutex);
					// 	buy_orderbook.AddtoBookwithTimeStamp(input, time, orderIdsToInstrumentsMap);
					// 	Output::OrderAdded(input.order_id, input.instrument,input.price,input.count,true,time);
					// }
				}
				break;
			}
			case input_sell:{
				// get the instrument - for cancel, this is an empty instrumentName
				std::shared_ptr<Instrument> instrumentPtr = bookShelf.getInstrumentBooksIfExistOrElseAddAndGet(std::string(input.instrument));
                OrderBook& buy_orderbook = instrumentPtr->buyBook;
				// simply add to sell orderbook if the buy orderbook is empty, need to acquire sell mutex
				if (buy_orderbook.books.empty()){
					const std::lock_guard<std::mutex> lock{instrumentPtr->instrument_sell_book_mutex};
					chrono_reps time = getCurrentTimestamp();
					(instrumentPtr->sellBook).AddtoBookwithTimeStamp(input,time, orderIdsToInstrumentsMap);
					Output::OrderAdded(input.order_id, input.instrument,input.price,input.count,true,time);
				}
				else {
					// acquire the buy mutex
					const std::lock_guard<std::mutex> lock{instrumentPtr->instrument_buy_book_mutex};
					buy_orderbook.SortOrders();
					buy_orderbook.MatchOrders(input, orderIdsToInstrumentsMap);
					chrono_reps output_time = getCurrentTimestamp();

					// loop through the books for matched orders
					for (auto it = buy_orderbook.books.rbegin(); it != buy_orderbook.books.rend(); it++){
						if((*it).matched == true){
							Output::OrderExecuted(input.order_id,(*it).order_id,(*it).execution_ID,
							(*it).price,(*it).count,output_time);
							(*it).matched = false; // rest matched state
							(*it).time_stamp = output_time; // update the timestamps, for cancelling orders 
						} /*else {
							break;
						}*/
					}

					// check if input order has been fully flled, add to sell book if not, need to acquire sell mutex here
					if(input.count > 0) {
                        const std::lock_guard<std::mutex> lock{instrumentPtr->instrument_sell_book_mutex};
						chrono_reps time = getCurrentTimestamp();
						(instrumentPtr->sellBook).AddtoBookwithTimeStamp(input, time, orderIdsToInstrumentsMap);
					}
				}
				break;
			}
			case input_cancel: {
				SyncCerr {} << "Got cancel: ID: " << input.order_id << std::endl;
				auto output_time = getCurrentTimestamp(); // shall we protect the time variable as well?
				SyncCerr {} << "current timestamp: " << output_time << std::endl;
				
				// is there a mutex supposed to be here? There is no guarantee on cancelled order right, if another B/S thread comes at the same time and is served first?
				std::string instrumentName = getInstrumentForOrderId(input.order_id);

				std::shared_ptr<Instrument> instrumentPtr = bookShelf.getInstrumentBooksIfExistOrElseAddAndGet(instrumentName);
				
				bool cancel_in_buy = false;
				bool cancel_in_sell = false;
				{
					// acquire the buy mutex
					const std::lock_guard<std::mutex> lock{instrumentPtr->instrument_buy_book_mutex};
					(instrumentPtr->buyBook).QueryAndCancelOrder(input, output_time, cancel_in_buy, orderIdsToInstrumentsMap);
					// std::cout <<"is it cancelled in buy? " << cancel_in_buy <<std::endl;
				}

				{
					// acquire the sell mutex
					const std::lock_guard<std::mutex> lock{instrumentPtr->instrument_sell_book_mutex};
                    (instrumentPtr->sellBook).QueryAndCancelOrder(input, output_time,cancel_in_sell, orderIdsToInstrumentsMap);
					// std::cout <<"is it cancelled in sell? " << cancel_in_sell <<std::endl;
				}
				// either one success would call the following output
				Output::OrderDeleted(input.order_id, (cancel_in_buy || cancel_in_sell), output_time);
				break;
			}

			default: {
				// SyncCerr {}
				//     << "Got order: " << static_cast<char>(input.type) << " " << input.instrument << " x " << input.count << " @ "
				//     << input.price << " ID: " << input.order_id << std::endl;

				// Remember to take timestamp at the appropriate time, or compute
				// an appropriate timestamp!
				// chrono_reps output_time = getCurrentTimestamp();
				// Output::OrderAdded(input.order_id, input.instrument, input.price, input.count, input.type == input_sell,
				//     output_time);
				break;
			}
		}
		// std::cout <<"should release the locks..."<<std::endl;


		// Additionally:

		// Remember to take timestamp at the appropriate time, or compute
		// an appropriate timestamp!
		// chrono_reps output_time = getCurrentTimestamp();

		// Check the parameter names in `io.hpp`.
		//Output::OrderExecuted(123, 124, 1, 2000, 10, output_time);
	}
}

