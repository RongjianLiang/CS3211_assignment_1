#include "RestOrder.cpp"
#include <algorithm>

class OrderBook
{
public:
    std::vector<RestOrder>books;
    char* inst_type[9]; // orders inside this book shall have same instrument
    std::string type;			// orders inside this book shall have same type

	// Default constructor
	OrderBook(){
        std::cout << "ob" << std::endl;
		//this->type = type;
	};

	void setInst (char* inst){
		std::memcpy(&(this->type), inst, 9);
	}

	// add order from client command and time-stamping
	void AddtoBookwithTimeStamp(ClientCommand& input, uint32_t time){
		// this section should be protected 
		std::cout << "inside adder..."<<std::endl;
		RestOrder order = RestOrder(input); // the RestOrder constructor should time-stamp
		order.time_stamp = time;
		books.push_back(order); 
		std::cout << "exiting adder..." <<std::endl;
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
	void QueryAndCancelOrder(ClientCommand input, uint32_t time_stamp, bool& res){
		bool result = false;
		int index = 0;

		for(auto it = this->books.begin(); it != this->books.end(); it++){
			if((*it).order_id == input.order_id){
				// compare the timestamp and check if the order is fully filled 
				std::cout << "timestamp check: "<< ((*it).time_stamp < time_stamp) <<std::endl;
				std::cout << "count check: "<< ((*it).count > 0) << std::endl;
				result = (((*it).time_stamp < time_stamp) && ((*it).count > 0)); 
				std::cout << "result: " << result << std::endl;
				break;
			}
			index ++;
		}
		// purge the order
		if (result == true){
			this->books.erase(books.begin() + index);
		}
		res = result;
	}
};

