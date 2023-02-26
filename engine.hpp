// This file contains declarations for the main Engine class. You will
// need to add declarations to this file as you develop your Engine.

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <chrono>
#include <iostream>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>

#include "BookShelf.hpp"
#include "io.hpp"
#include <atomic>

static std::atomic<int> timesta;

// a timstamping scheme should be implemented here?
inline chrono_reps getCurrentTimestamp() noexcept
{
	return timesta.fetch_add(1);
// 	return std::floor(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

struct Engine
{
public:
	void accept(ClientConnection conn);

private:
    BookShelf bookShelf{};
   	std::unordered_map<uint32_t, std::string> orderIdsToInstrumentsMap;

	std::string getInstrumentForOrderId(uint32_t order_id);
    void connection_thread(ClientConnection conn);
};

#endif
