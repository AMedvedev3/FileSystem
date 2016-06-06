#pragma once
/**
@file
Trace logging definitions
@author Alexey Medvedev
*/
#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

// TODO: configure the logger better, setup file, debug and console, set switching filter level
BOOST_LOG_GLOBAL_LOGGER(logger, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>)
#define LOG(severity) BOOST_LOG_SEV(logger::get(),boost::log::trivial::severity)
// log macroses
#define LF     LOG(info)//BOOST_LOG_TRIVIAL(info)
#define LE     LOG(error)

// TODO: change this value to filter level of log messages
static int g_level = boost::log::trivial::severity_level::error;

//void init_log();
