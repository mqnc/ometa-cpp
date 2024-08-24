#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include "view.h"

namespace ometa {

size_t logIndent = 0;
const char* red = "\x1b[91m";
const char* green = "\x1b[92m";
const char* yellow = "\x1b[93m";
const char* blue = "\x1b[94m";
const char* magenta = "\x1b[95m";
const char* cyan = "\x1b[96m";
const char* reset = "\x1b[0m";

enum class LogEvent {
	enter, accept, reject
};

template <forward_range TSource>
void log(
	std::string name,
	LogEvent event,
	View<TSource> src,
	View<TSource> next = {}
) {

	if (event == LogEvent::accept || event == LogEvent::reject) {
		logIndent--;
	}

	for (size_t i = 0; i < logIndent; i++) { std::cout << blue << "|  "; }
	std::cout << magenta << name;
	switch (event) {
		case LogEvent::enter: std::cout << yellow << "[?] "; break;
		case LogEvent::accept: std::cout << green << "[√] »"; break;
		case LogEvent::reject: std::cout << red << "[X] "; break;
	}
	std::cout << reset;

	auto printFirstChar = [next](const auto& src) {
		if (src.begin() == next.begin()) {
			std::cout << green << "«" << reset;
		}
		if (src.begin() != src.end()) {
			std::stringstream ss;
			ss << src.front();
			if (ss.str() == " ") {
				std::cout << blue << "·" << reset;
			}
			else if (ss.str() == "\t") {
				std::cout << blue << "→" << reset;
			}
			else if (ss.str() == "\n") {
				std::cout << blue << "↲" << reset;
			}
			else {
				std::cout << ss.str();
			}
			return true;
		}
		else {
			std::cout << blue << "∎" << reset;
			return false;
		}
	};

	// const size_t s = src.size();
	// if (s <= 30) {
	size_t i = 0;
	while (printFirstChar(src) && i < 30) {
		src.next();
		i++;
	}
	// }
	// else {
	// 	for (size_t i = 0; i < 15; i++) {
	// 		printFirstChar(src);
	// 		src.next();
	// 	}

	// }
	std::cout << "\n";

	if (event == LogEvent::enter) { logIndent++; }
}

#ifdef DEBUG_PRINTS
	#define OMETA_LOG(p) (p).name = #p
#else
	#define OMETA_LOG(p)
#endif

}
