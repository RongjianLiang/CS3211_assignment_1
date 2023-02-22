class Instrument
{
    std::mutex instrument_buy_book_mutex;
    std::mutex instrument_sell_book_mutex;
    OrderBook sellBook;
    OrderBook buyBook;   
 
    OrderBook& getInstrumentSellBook(ClientCommand input);
	
    OrderBook& getInstrumentBuyBook(ClientCommand input);
};

class BookShelf
{
    std::mutex get_instrument;
    std::unordered_map<std::string, Instrument> bookShelf;

public:
    Instrument& getInstrumentBooks(std::string instrument);
};
