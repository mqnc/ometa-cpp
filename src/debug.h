#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include "sourceview.h"

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
void log(std::string name, SourceView<TSource> src, LogEvent event) {

	if (event == LogEvent::accept || event == LogEvent::reject) {
		logIndent--;
	}

	for (size_t i = 0; i < logIndent; i++) { std::cout << blue << "|  "; }
	std::cout << magenta << name;
	switch (event) {
		case LogEvent::enter: std::cout << yellow << "[?] "; break;
		case LogEvent::accept: std::cout << green << "[√] "; break;
		case LogEvent::reject: std::cout << red << "[X] "; break;
	}
	std::cout << reset;
	for (size_t i = 0; i < 20; i++) {
		std::stringstream ss;
		if (src.begin() != src.end()) {
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
		}

		else {
			std::cout << blue << "∎";
			break;
		}
		src.next();
	}
	std::cout << reset << "\n";

	if (event == LogEvent::enter) { logIndent++; }
}

}
