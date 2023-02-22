#include <string>

#include "BookShelf.hpp"

OrderBook& Instrument::getInstrumentSellBook(ClientCommand& input)
{
    const std::lock_guard<std::mutex> lock (instrument_sell_book_mutex);
    return sellBook;
}
	
OrderBook& Instrument::getInstrumentBuyBook(ClientCommand& input)
{
	const std::lock_guard<std::mutex> lock (instrument_buy_book_mutex);
    return buyBook;
}

std::shared_ptr<Instrument> BookShelf::getInstrumentBooks(std::string instrumentName) {
    const std::shared_lock lock(bookshelf_mutex);

    if (bookShelf.contains(instrumentName)) {
        return bookShelf.at(std::string(instrumentName));
    }
    else {
        return std::shared_ptr<Instrument>{};
    }
}

void BookShelf::addInstrumentBooks(std::string instrumentName) {
    const std::shared_lock lock{(bookshelf_mutex)};

    if (bookShelf.contains(instrumentName)) {
        return;
    }
    std::shared_ptr<Instrument> newInstrument = std::make_shared<Instrument>();
    auto pair = std::make_pair(instrumentName, newInstrument);
    bookShelf.insert(pair);
}

std::shared_ptr<Instrument> BookShelf::getInstrumentBooksIfExistOrElseAddAndGet(std::string instrumentName) {
    std::shared_ptr<Instrument> instrumentBooks = getInstrumentBooks(instrumentName);
    if (instrumentBooks) {
        addInstrumentBooks(instrumentName);
        return getInstrumentBooks(instrumentName);
    }
    return instrumentBooks;
}

