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

Instrument* BookShelf::getInstrumentBooks(std::string instrumentName) {
    const std::shared_lock lock(bookshelf_mutex);

    if (bookShelf.contains(instrumentName)) {
        return &(bookShelf.at(std::string(instrumentName)));
    }
    else {
        return NULL;
    }
}

void BookShelf::addInstrumentBooks(std::string instrumentName) {
    const std::unique_lock lock{(bookshelf_mutex)};

    if (bookShelf.contains(instrumentName)) {
        return;
    }
    Instrument newInstrument{};
    std::string temp = instrumentName;
    auto pzz = std::make_pair(instrumentName, std::move(newInstrument));
    bookShelf.insert({temp, newInstrument});
}

Instrument& BookShelf::getInstrumentBooksIfExistOrElseAddAndGet(std::string instrumentName) {
    Instrument* instrumentBooks = getInstrumentBooks(instrumentName);
    if (instrumentBooks == NULL) {
        addInstrumentBooks(instrumentName);
        return *(getInstrumentBooks(instrumentName));
    }
    return *(instrumentBooks);
}

