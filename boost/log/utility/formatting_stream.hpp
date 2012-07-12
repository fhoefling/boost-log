/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   formatting_stream.hpp
 * \author Andrey Semashev
 * \date   11.07.2012
 *
 * The header contains implementation of a string stream used for log record formatting.
 */

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_FORMATTING_STREAM_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_FORMATTING_STREAM_HPP_INCLUDED_

#include <ostream>
#include <string>
#include <memory>
#include <locale>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
#include <boost/log/detail/code_conversion.hpp>
#include <boost/log/utility/string_literal_fwd.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

template<
    typename CharT,
    typename TraitsT = std::char_traits< CharT >,
    typename AllocatorT = std::allocator< CharT >
>
class basic_formatting_ostream :
    private boost::log::aux::basic_ostringstreambuf< CharT, TraitsT, AllocatorT >,
    public std::basic_ostream< CharT, TraitsT >
{
public:
    //! Character type
    typedef CharT char_type;
    //! Character traits
    typedef TraitsT traits_type;
    //! Memory allocator
    typedef AllocatorT allocator_type;
    //! Stream buffer type
    typedef boost::log::aux::basic_ostringstreambuf< char_type, traits_type, allocator_type > streambuf_type;
    //! Target string type
    typedef typename streambuf_type::string_type string_type;

    //! Stream type
    typedef std::basic_ostream< char_type, traits_type > ostream_type;
    //! Stream position type
    typedef typename ostream_type::pos_type pos_type;
    //! Stream offset type
    typedef typename ostream_type::off_type off_type;

private:
    //  Supported character types
    typedef mpl::vector<
        char
      , signed char
      , unsigned char
      , wchar_t
#ifndef BOOST_NO_CHAR16_T
      , char16_t
#endif
#ifndef BOOST_NO_CHAR32_T
      , char32_t
#endif
    >::type char_types;

    //  Function types
    typedef std::ios_base& (*ios_base_manip)(std::ios_base&);
    typedef std::basic_ios< char_type, traits_type >& (*basic_ios_manip)(std::basic_ios< char_type, traits_type >&);
    typedef ostream_type& (*stream_manip)(ostream_type&);

public:
    /*!
     * Default constructor. Creates an empty record that is equivalent to the invalid record handle.
     * The stream capability is not available after construction.
     *
     * \post <tt>!*this == true</tt>
     */
    basic_formatting_ostream() : ostream_type(static_cast< ostream_buf_base_type* >(this))
    {
        init_stream();
    }

    /*!
     * Conversion from a record handle. Adopts the record referenced by the handle.
     *
     * \pre The handle, if valid, have been issued by the logging core with the same character type as the record being constructed.
     * \post <tt>this->handle() == rec</tt>
     * \param rec The record handle being adopted
     */
    explicit basic_formatting_ostream(string_type& str) :
        streambuf_type(str),
        ostream_type(static_cast< ostream_buf_base_type* >(this))
    {
        init_stream();
    }

    /*!
     * Destructor. Destroys the record, releases any sinks and attribute values that were involved in processing this record.
     */
    ~basic_formatting_ostream()
    {
        flush();
    }

    /*!
     * Attaches the stream to the string. The string will be used to store the formatted characters.
     *
     * \param str The string buffer to attach.
     */
    void attach(string_type& str)
    {
        streambuf_type::attach(str);
        ostream_type::clear(ostream_type::goodbit);
    }
    /*!
     * Detaches the stream from the string. Any buffered data is flushed to the string.
     */
    void detach()
    {
        streambuf_type::detach();
        ostream_type::clear(ostream_type::badbit);
    }

    //  Stream method overrides
    basic_formatting_ostream& flush()
    {
        ostream_type::flush();
        return *this;
    }

    basic_formatting_ostream& seekp(pos_type pos)
    {
        ostream_type::seekp(pos);
        return *this;
    }

    basic_formatting_ostream& seekp(off_type off, std::ios_base::seekdir dir)
    {
        ostream_type::seekp(off, dir);
        return *this;
    }

    basic_formatting_ostream& operator<< (ios_base_manip manip)
    {
        *static_cast< ostream_type* >(this) << manip;
        return *this;
    }
    basic_formatting_ostream& operator<< (basic_ios_manip manip)
    {
        *static_cast< ostream_type* >(this) << manip;
        return *this;
    }
    basic_formatting_ostream& operator<< (stream_manip manip)
    {
        *static_cast< ostream_type* >(this) << manip;
        return *this;
    }
    template< typename OtherCharT >
    typename enable_if< mpl::contains< char_types, OtherCharT >, basic_formatting_ostream& >::type
    operator<< (OtherCharT c)
    {
        put(c);
        return *this;
    }

    template< typename OtherCharT >
    typename enable_if< mpl::contains< char_types, typename boost::remove_cv< OtherCharT >::type >, basic_formatting_ostream& >::type
    operator<< (OtherCharT* p)
    {
        typedef typename boost::remove_cv< OtherCharT >::type other_char_type;
        write(p, static_cast< std::streamsize >(std::char_traits< other_char_type >::length(p)));
        return *this;
    }

    template< typename OtherCharT, unsigned int N >
    typename enable_if< mpl::contains< char_types, typename boost::remove_cv< OtherCharT >::type >, basic_formatting_ostream& >::type
    operator<< (OtherCharT (&p)[N])
    {
        typedef typename boost::remove_cv< OtherCharT >::type other_char_type;
        write(p, static_cast< std::streamsize >(std::char_traits< other_char_type >::length(p)));
        return *this;
    }

    template< typename OtherCharT, typename OtherTraitsT, typename OtherAllocatorT >
    typename enable_if< mpl::contains< char_types, OtherCharT >, basic_formatting_ostream& >::type
    operator<< (std::basic_string< OtherCharT, OtherTraitsT, OtherAllocatorT > const& str)
    {
        write(str.c_str(), static_cast< std::streamsize >(str.size()));
        return *this;
    }

    template< typename OtherCharT, typename OtherTraitsT >
    typename enable_if< mpl::contains< char_types, OtherCharT >, basic_formatting_ostream& >::type
    operator<< (basic_string_literal< OtherCharT, OtherTraitsT > const& str)
    {
        write(str.c_str(), static_cast< std::streamsize >(str.size()));
        return *this;
    }

    template< typename T >
    basic_formatting_ostream& operator<< (T const& value)
    {
        *static_cast< ostream_type* >(this) << value;
        return *this;
    }

    //  A special formatting for optional values
    template< typename T >
    basic_formatting_ostream& operator<< (boost::optional< T > const& value)
    {
        if (!!value)
            *this << value.get();
        return *this;
    }

    basic_formatting_ostream& put(char_type c)
    {
        ostream_type::put(c);
        return *this;
    }

    template< typename OtherCharT >
    typename enable_if< mpl::contains< char_types, OtherCharT >, basic_formatting_ostream& >::type
    put(OtherCharT c)
    {
        write(boost::addressof(c), 1);
        return *this;
    }

    basic_formatting_ostream& write(const char_type* p, std::streamsize size)
    {
        ostream_type::write(p, size);
        return *this;
    }

    template< typename OtherCharT >
    typename enable_if< mpl::contains< char_types, OtherCharT >, basic_formatting_ostream& >::type
    write(const OtherCharT* p, std::streamsize size)
    {
        flush();

        string_type* storage = streambuf_type::storage();
        BOOST_ASSERT(storage != NULL);
        aux::code_convert(p, static_cast< std::size_t >(size), *storage, this->getloc());

        return *this;
    }

private:
    void init_stream()
    {
        ostream_type::clear(streambuf_type::storage() ? ostream_type::goodbit : ostream_type::badbit);
        ostream_type::flags
        (
            ostream_type::dec |
            ostream_type::skipws |
            ostream_type::boolalpha // this differs from the default stream flags but makes logs look better
        );
        ostream_type::width(0);
        ostream_type::precision(6);
        ostream_type::fill(static_cast< char_type >(' '));
    }

    //! Copy constructor (closed)
    BOOST_LOG_DELETED_FUNCTION(basic_formatting_ostream(basic_formatting_ostream const& that))
    //! Assignment (closed)
    BOOST_LOG_DELETED_FUNCTION(basic_formatting_ostream& operator= (basic_formatting_ostream const& that))
};

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_FORMATTING_STREAM_HPP_INCLUDED_