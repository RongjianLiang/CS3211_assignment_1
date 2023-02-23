#include <vector>
#include <cstring>

#include "io.hpp"

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

	// Move constructor 
};

