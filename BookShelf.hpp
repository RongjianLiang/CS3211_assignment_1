#include <unordered_map>
#include <memory>
#include <shared_mutex>

#include "io.hpp"
#include "OrderBook.cpp"

class Instrument
{
private:
public:
    std::mutex instrument_buy_book_mutex;
    std::mutex instrument_sell_book_mutex;
    SellOrderBook sellBook = SellOrderBook();
    BuyOrderBook buyBook = BuyOrderBook();   
    
    Instrument() {}
};

class BookShelf
{
private:
    std::shared_mutex bookshelf_mutex;
    std::unordered_map<std::string, std::shared_ptr<Instrument>> bookShelf;

    void addInstrumentBooks(std::string instrumentName);
    std::shared_ptr<Instrument> getInstrumentBooks(std::string instrumentName);

public:
    std::shared_ptr<Instrument> getInstrumentBooksIfExistOrElseAddAndGet(std::string instrumentName);
};

