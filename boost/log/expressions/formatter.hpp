/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   formatter.hpp
 * \author Andrey Semashev
 * \date   13.07.2012
 *
 * The header contains a formatter function object definition.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_FORMATTER_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_FORMATTER_HPP_INCLUDED_

#include <boost/move/move.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/detail/light_function.hpp>
#include <boost/log/detail/functional.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/utility/formatting_stream.hpp>
#include <boost/log/expressions/message.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * Log record formatter function wrapper.
 */
template< typename CharT >
class basic_formatter
{
    typedef basic_formatter this_type;
    BOOST_COPYABLE_AND_MOVABLE(this_type)

public:
    //! Result type
    typedef void result_type;

    //! Character type
    typedef CharT char_type;
    //! Output stream type
    typedef basic_formatting_ostream< char_type > stream_type;

private:
    //! Filter function type
    typedef boost::log::aux::light_function2< void, record const&, stream_type& > formatter_type;

    //! Default formatter, always returns \c true
    struct default_formatter
    {
        typedef void result_type;

        default_formatter() : m_MessageName(expressions::tag::message::get_name())
        {
        }

        result_type operator() (record const& rec, stream_type& strm) const
        {
            boost::log::visit< expressions::tag::message::value_type >(m_MessageName, rec, boost::log::aux::output_fun< stream_type >(strm));
        }

    private:
        const attribute_name m_MessageName;
    };

private:
    //! Formatter function
    formatter_type m_Formatter;

public:
    /*!
     * Default constructor. Creates a formatter that only outputs log message.
     */
    basic_formatter() : m_Formatter(default_formatter())
    {
    }
    /*!
     * Copy constructor
     */
    basic_formatter(basic_formatter const& that) : m_Formatter(that.m_Formatter)
    {
    }
    /*!
     * Move constructor
     */
    basic_formatter(BOOST_RV_REF(this_type) that) : m_Formatter(boost::move(that.m_Formatter))
    {
    }

    /*!
     * Initializing constructor. Creates a formatter which will invoke the specified function object.
     */
#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    template< typename FunT >
    basic_formatter(FunT const& fun)
#else
    template< typename FunT >
    basic_formatter(FunT const& fun, typename disable_if< move_detail::is_rv< FunT >, int >::type = 0)
#endif
        : m_Formatter(fun)
    {
    }

    /*!
     * Move assignment.
     */
    basic_formatter& operator= (BOOST_RV_REF(this_type) that)
    {
        m_Formatter.swap(that.m_Formatter);
        return *this;
    }
    /*!
     * Copy assignment.
     */
    basic_formatter& operator= (BOOST_COPY_ASSIGN_REF(this_type) that)
    {
        m_Formatter = that.m_Formatter;
        return *this;
    }
    /*!
     * Initializing assignment. Sets the specified function object to the formatter.
     */
#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    template< typename FunT >
    basic_formatter& operator= (FunT const& fun)
#else
    template< typename FunT >
    typename disable_if< is_same< typename remove_cv< FunT >::type, this_type >, this_type& >::type
    operator= (FunT const& fun)
#endif
    {
        this_type(fun).swap(*this);
        return *this;
    }

    /*!
     * Formatting operator.
     *
     * \param rec A log record to format.
     * \param strm A stream to put the formatted characters to.
     */
    result_type operator() (record const& rec, stream_type& strm) const
    {
        m_Formatter(rec, strm);
    }

    /*!
     * Resets the formatter to the default. The default formatter only outputs message text.
     */
    void reset()
    {
        m_Formatter = default_formatter();
    }

    /*!
     * Swaps two formatters
     */
    void swap(basic_formatter& that)
    {
        m_Formatter.swap(that.m_Formatter);
    }
};

template< typename CharT >
inline void swap(basic_formatter< CharT >& left, basic_formatter< CharT >& right)
{
    left.swap(right);
}

#ifdef BOOST_LOG_USE_CHAR
typedef basic_formatter< char > formatter;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_formatter< wchar_t > wformatter;
#endif

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_FORMATTER_HPP_INCLUDED_
