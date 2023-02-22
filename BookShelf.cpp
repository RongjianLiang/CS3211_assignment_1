#include <string>

#include "BookShelf.hpp"

OrderBook& getInstrumentSellBook(ClientCommand& input)
{
    const std::lock_guard<std::mutex> lock (instrument_sell_book_mutex);
    return sellBook;
}
	
OrderBook& getInstrumentBuyBook(ClientCommand& input)
{
	const std::lock_guard<std::mutex> lock (instrument_buy_book_mutex);
    return buyBook;
}

Instrument& getInstrumentBooks(std::string instrument) {
	const std::lock_guard<std::mutex> lock (get_instrument);
    if (bookShelf.contains(std::string(input.instrument)) {
        return bookShelf.at(std::string(input.instrument));
    }
    else {
        Instrument newInstrument();
        bookShelf.insert({instrument, newInstrument});
        return newInstrument;
    }
}
