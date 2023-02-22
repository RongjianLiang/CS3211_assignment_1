class BookShelf
{
    std::unordered_map<std::string, Instrument&> myMap;

public:
	void addToShelf (OrderBook orderbook){
		shelf.push_back(orderbook);
	}

	OrderBook& getInstrumentSellBook(ClientCommand input){
	}
	
    OrderBook& getInstrumentBuyBook(ClientCommand input){
	}
};
