#include "RestOrder.cpp"
#include <algorithm>
#include <unordered_map>

class OrderBook
{
public:
    std::vector<RestOrder>books;
	
	// add order from client command and time-stamping
	void AddtoBookwithTimeStamp(ClientCommand& input, uint32_t time, std::unordered_map<uint32_t, std::string>& orderIdsToInstrumentsMap){
		// this section should be protected 
		RestOrder order = RestOrder(input); // the RestOrder constructor should time-stamp
		order.time_stamp = time;
		
		books.push_back(order);
		orderIdsToInstrumentsMap.insert({input.order_id, std::string(input.instrument)});
	};

	// sort the vector contains orders
	virtual void SortOrders() = 0; 
 
	virtual void MatchOrders(ClientCommand& input) = 0;
	
	// false if rejected, either deleted or not-exist
	// attempt to cancel deleted order would simply return false, as no such match exists. 
	// the timestamp is determined in engine.cpp 
	void QueryAndCancelOrder(ClientCommand input, uint32_t time_stamp, bool& res, std::unordered_map<uint32_t, std::string>& orderIdsToInstrumentsMap){
		bool result = false;
		int index = 0;
		
		for(auto it = this->books.begin(); it != this->books.end(); it++){
			if((*it).order_id == input.order_id){
				// compare the timestamp and check if the order is fully filled 
				result = (((*it).time_stamp < time_stamp) && ((*it).count == 0));
				std::cout << (*it).count << std::endl;
				break;
			}
			index++;
		}
		// purge the order
		if (result == true){
			this->books.erase(books.begin() + index);
			orderIdsToInstrumentsMap.erase(input.order_id);
		}
		res = result;
	}
};

class BuyOrderBook: public OrderBook {
public: 
	void MatchOrders(ClientCommand& input) {
		for(auto it = this->books.begin(); it != this->books.end(); it++){
			// B price > S price
			if ((*it).price >= input.price && input.count > 0){
				(*it).execution_ID ++;
				(*it).matched = true;
				// new order fully filled, with resting order fully or partially filled
				if(input.count <= (*it).count){
				    (*it).trades = input.count;
				    (*it).count -= input.count;
				    input.count = 0;
				}
				// new order partially or fully filed, with resting order fully filled 
				else if(input.count >= (*it).count){
				    (*it).trades = (*it).count;
				    input.count -= (*it).count;
				    (*it).count = 0;
				}
			} 
		}
	}

	void SortOrders (){ 
		// sort by price & check validity
		std::sort(this->books.begin(), this->books.end(),[](const RestOrder& a, const RestOrder& b)
		{
			if(a.price != b.price) {
				// orderbook is all buying orders, sort the price in descending order
				return a.price > b.price;
			}
			else { // if same price, sort the time stamp in descending order
				return a.time_stamp < b.time_stamp;
			}
		});
	}
};

class SellOrderBook: public OrderBook {
public: 
	void MatchOrders (ClientCommand& input) {
		for(auto it = this->books.begin(); it != this->books.end(); it++){
			// B price > S price
			if ((*it).price <= input.price && input.count > 0) {
				(*it).execution_ID ++;
				(*it).matched = true;
				// new order fully filled, with resting order fully or partially filled
				if(input.count <= (*it).count){
				    (*it).trades = input.count;
                    (*it).count -= input.count;
				    input.count = 0;
				}
				// new order partially or fully filed, with resting order fully filled 
				else if(input.count >= (*it).count){
				    (*it).trades = (*it).count;
				    input.count -= (*it).count;
				    (*it).count = 0;
				}
			} 

		}
	}

	void SortOrders (){ 
		// sort by price & check validity
		std::sort(this->books.begin(), this->books.end(),[](const RestOrder& a, const RestOrder& b)
		{
			if(a.price != b.price) {
				// orderbook is all selling orders, sort the price in ascending order
				return a.price < b.price;
			}
			else { // if same price, sort the time stamp in descending order
				return a.time_stamp < b.time_stamp;
			}
		});
	}
};

