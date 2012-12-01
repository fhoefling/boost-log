/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   trivial_keyword.hpp
 * \author Andrey Semashev
 * \date   02.12.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#ifndef BOOST_LOG_DETAIL_TRIVIAL_KEYWORD_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_TRIVIAL_KEYWORD_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace trivial {

//! Trivial severity keyword
BOOST_LOG_ATTRIBUTE_KEYWORD("Severity", severity, severity_level)

} // namespace trivial

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_TRIVIAL_KEYWORD_HPP_INCLUDED_