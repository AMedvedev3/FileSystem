/**
@file
Trace logging implementation
@author Alexey Medvedev
*/
#include "log.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/debug_output_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost\log\utility\setup\console.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;


BOOST_LOG_GLOBAL_LOGGER_INIT(logger, src::severity_logger_mt)
{
        src::severity_logger_mt<boost::log::trivial::severity_level> logger;
        boost::shared_ptr< logging::core > core = logging::core::get();
    
    #ifdef DEBUG_VIEW_LOG
        // Create the sink. The backend requires synchronization in the frontend.
        boost::shared_ptr< sinks::synchronous_sink< sinks::debug_output_backend > > 
            sink(new sinks::synchronous_sink< sinks::debug_output_backend >());
    
        // Set the special filter to the frontend
        // in order to skip the sink when no debugger is available
        sink->set_filter(expr::is_debugger_present());
    
        core->add_sink(sink);
    #endif
    
    #ifdef CONSOLE_LOG
    
        logging::add_console_log(std::clog,
            keywords::format = "[%TimeStamp%]: %Message%");
    
    #endif
    
    #ifdef FILE_LOG
        logging::add_file_log
        (
            keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
            keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
            keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
        );
    #endif
        core->set_filter
        (
            logging::trivial::severity >= g_level
        );

        return logger;
}

//void init_log()
//{
//    // Add output to DebugView
//    boost::shared_ptr< logging::core > core = logging::core::get();
//
//#ifdef DEBUG_VIEW_LOG
//    // Create the sink. The backend requires synchronization in the frontend.
//    boost::shared_ptr< sinks::synchronous_sink< sinks::debug_output_backend > > 
//        sink(new sinks::synchronous_sink< sinks::debug_output_backend >());
//
//    // Set the special filter to the frontend
//    // in order to skip the sink when no debugger is available
//    sink->set_filter(expr::is_debugger_present());
//
//    core->add_sink(sink);
//#endif
//
//#ifdef CONSOLE_LOG
//
//    logging::add_console_log(std::clog,
//        keywords::format = "[%TimeStamp%]: %Message%");
//
//#endif
//
//#ifdef FILE_LOG
//    logging::add_file_log
//    (
//        keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
//        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
//        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
//        keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
//    );
//#endif
//    core->set_filter
//    (
//        logging::trivial::severity >= logging::trivial::info
//    );
//    
//}