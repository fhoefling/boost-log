/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   default_sink.hpp
 * \author Andrey Semashev
 * \date   08.01.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DEFAULT_SINK_HPP_INCLUDED_
#define BOOST_LOG_DEFAULT_SINK_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/expressions/message.hpp>
#include <boost/log/trivial.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/mutex.hpp>
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace sinks {

namespace aux {

//! The default sink to be used when no sinks are registered in the logging core
class default_sink :
    public sink
{
private:
#if !defined(BOOST_LOG_NO_THREADS)
    typedef mutex mutex_type;
    mutex_type m_mutex;
#endif
    value_extractor< extract_value_or_default< boost::log::trivial::severity_level > > const m_severity_extractor;
    value_visitor_invoker< expressions::tag::message::value_type > m_message_visitor;

public:
    default_sink();
    ~default_sink();
    bool will_consume(attribute_values_view const&);
    void consume(record const& rec);
    void flush();
};

} // namespace aux

} // namespace sinks

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DEFAULT_SINK_HPP_INCLUDED_
