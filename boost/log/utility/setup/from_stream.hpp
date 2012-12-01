/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   from_stream.hpp
 * \author Andrey Semashev
 * \date   22.03.2008
 *
 * The header contains definition of facilities that allows to initialize the library from a
 * settings file.
 */

#ifndef BOOST_LOG_UTILITY_SETUP_FROM_STREAM_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_SETUP_FROM_STREAM_HPP_INCLUDED_

#include <iosfwd>
#include <boost/log/detail/setup_prologue.hpp>

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * The function initializes the logging library from a stream containing logging settings
 *
 * \param strm Stream, that provides library settings
 *
 * \b Throws: An <tt>std::exception</tt>-based exception if the read data cannot be interpreted as the library settings
 */
template< typename CharT >
BOOST_LOG_SETUP_API void init_from_stream(std::basic_istream< CharT >& strm);

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_SETUP_FROM_STREAM_HPP_INCLUDED_