#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std;

void initLogging();
shared_ptr<spdlog::logger> logger();

#endif // LOGGER_HPP