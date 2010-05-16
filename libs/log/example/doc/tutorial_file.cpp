/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/filters.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/init/to_file.hpp>
#include <boost/log/utility/init/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace flt = boost::log::filters;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

#if 0

//[ example_tutorial_file_simple
void init()
{
    logging::init_log_to_file("sample.log");
    logging::core::get()->set_filter
    (
        flt::attr< logging::trivial::severity_level >("Severity") >= logging::trivial::info
    );
}
//]

#else

//[ example_tutorial_file_advanced
void init()
{
    logging::init_log_to_file
    (
        keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
        keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
        keywords::format = "[%TimeStamp%]: %_%"                                       /*< log record format >*/
    );

    logging::core::get()->set_filter
    (
        flt::attr< logging::trivial::severity_level >("Severity") >= logging::trivial::info
    );
}
//]

#endif

int main(int, char*[])
{
    init();
    logging::add_common_attributes();

    using namespace logging::trivial;
    src::severity_logger< severity_level > lg;

    BOOST_LOG_SEV(lg, trace) << "A trace severity message";
    BOOST_LOG_SEV(lg, debug) << "A debug severity message";
    BOOST_LOG_SEV(lg, info) << "An informational severity message";
    BOOST_LOG_SEV(lg, warning) << "A warning severity message";
    BOOST_LOG_SEV(lg, error) << "An error severity message";
    BOOST_LOG_SEV(lg, fatal) << "A fatal severity message";

    return 0;
}