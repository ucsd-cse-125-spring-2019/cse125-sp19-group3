#include "logger.hpp"

void initLogging() {
	auto err_logger = spdlog::stderr_color_mt("stderr");
	spdlog::set_level(spdlog::level::debug);
	spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] [thread %t] %v");
}

shared_ptr<spdlog::logger> logger() {
	return spdlog::get("stderr");
}