#include <unordered_map>

#include "io.hpp"
#include "OrderBook.cpp"

class Instrument
{
private:
    std::mutex instrument_buy_book_mutex;
    std::mutex instrument_sell_book_mutex;
    OrderBook sellBook;
    OrderBook buyBook;   
public:
    Instrument(): sellBook(), buyBook() {}
 
    OrderBook& getInstrumentSellBook(ClientCommand& input);
	
    OrderBook& getInstrumentBuyBook(ClientCommand& input);
};

class BookShelf
{
private:
    std::shared_mutex bookshelf_mutex;
    std::unordered_map<std::string, Instrument> bookShelf;

    void addInstrumentBooks(std::string instrumentName);
    Instrument* getInstrumentBooks(std::string instrumentName);

public:
    Instrument& getInstrumentBooksIfExistOrElseAddAndGet(std::string instrumentName);
};

