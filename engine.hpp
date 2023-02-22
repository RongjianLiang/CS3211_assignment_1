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
#include <shared_mutex>

#include "BookShelf.hpp"
#include "io.hpp"

// a timstamping scheme should be implemented here?
inline std::chrono::microseconds::rep getCurrentTimestamp() noexcept
{
	return std::floor(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
	/ 1000000);
}

struct Engine
{
public:
	void accept(ClientConnection conn);

private:
    BookShelf bookShelf();
    
    void connection_thread(ClientConnection conn);
};

#endif
