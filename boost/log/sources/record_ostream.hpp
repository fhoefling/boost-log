/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   record_ostream.hpp
 * \author Andrey Semashev
 * \date   09.03.2009
 *
 * This header contains a wrapper class around a logging record that allows to compose the
 * record message with a streaming expression.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SOURCES_RECORD_OSTREAM_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_RECORD_OSTREAM_HPP_INCLUDED_

#include <new>
#include <string>
#include <ostream>
#include <boost/utility/addressof.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/native_typeof.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/utility/unique_identifier_name.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>
#include <boost/log/utility/formatting_stream.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
// 'this' : used in base member initializer list
#pragma warning(disable: 4355)
#endif // _MSC_VER

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * \brief Logging record adapter with a streaming capability
 *
 * This class allows to compose the logging record message by streaming operations. It
 * aggregates the log record and provides the standard output stream interface.
 */
template< typename CharT >
class basic_record_ostream :
    public basic_formatting_ostream< CharT >
{
    //! Self type
    typedef basic_record_ostream< CharT > this_type;
    //! Base stream class
    typedef basic_formatting_ostream< CharT > base_type;

public:
    //! Character type
    typedef CharT char_type;
    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Stream type
    typedef std::basic_ostream< char_type > stream_type;

private:
    //! Log record
    record m_Record;

public:
    /*!
     * Default constructor. Creates an empty record that is equivalent to the invalid record handle.
     * The stream capability is not available after construction.
     *
     * \post <tt>!*this == true</tt>
     */
    BOOST_LOG_DEFAULTED_FUNCTION(basic_record_ostream(), {})

    /*!
     * Conversion from a record object. Adopts the record referenced by the object.
     *
     * \pre The handle, if valid, have been issued by the logging core with the same character type as the record being constructed.
     * \post <tt>this->handle() == rec</tt>
     * \param rec The record handle being adopted
     */
    basic_record_ostream(record const& rec) :
        m_Record(rec)
    {
        init_stream();
    }

    /*!
     * Destructor. Destroys the record, releases any sinks and attribute values that were involved in processing this record.
     */
    ~basic_record_ostream()
    {
        detach_from_record();
    }

    /*!
     * Conversion to an unspecified boolean type
     *
     * \return \c true, if stream is valid and ready for formatting, \c false, if the stream is not valid. The latter also applies to
     *         the case when the stream is not attached to a log record.
     */
    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()

    /*!
     * Inverted conversion to an unspecified boolean type
     *
     * \return \c false, if stream is valid and ready for formatting, \c true, if the stream is not valid. The latter also applies to
     *         the case when the stream is not attached to a log record.
     */
    bool operator! () const
    {
        return (!m_Record || base_type::fail());
    }

    /*!
     * Flushes internal buffers to complete all pending formatting operations and returns the aggregated log record
     *
     * \return The aggregated record object
     */
    record const& get_record() const
    {
        const_cast< this_type* >(this)->flush();
        return m_Record;
    }

    /*!
     * If the stream is attached to a log record, flushes internal buffers to complete all pending formatting operations.
     * Then reattaches the stream to another log record.
     *
     * \param rec New log record to attach to
     */
    void set_record(record rec)
    {
        detach_from_record();
        m_Record.swap(rec);
        init_stream();
    }

    //! The function resets the stream into a detached (default initialized) state
    BOOST_LOG_API void detach_from_record();

private:
    //! The function initializes the stream and the stream buffer
    BOOST_LOG_API void init_stream();

    //  Copy and assignment are closed
    BOOST_LOG_DELETED_FUNCTION(basic_record_ostream(basic_record_ostream const&))
    BOOST_LOG_DELETED_FUNCTION(basic_record_ostream& operator= (basic_record_ostream const&))
};


#ifdef BOOST_LOG_USE_CHAR
typedef basic_record_ostream< char > record_ostream;        //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_record_ostream< wchar_t > wrecord_ostream;    //!< Convenience typedef for wide-character logging
#endif

namespace aux {

//! Internal class that provides formatting streams for record pumps
template< typename CharT >
struct stream_provider
{
    //! Character type
    typedef CharT char_type;

    //! Formatting stream compound
    struct stream_compound
    {
        stream_compound* next;

        //! Log record stream adapter
        basic_record_ostream< char_type > stream;

        //! Initializing constructor
        explicit stream_compound(record const& rec) : next(NULL), stream(rec) {}
    };

    //! The method returns an allocated stream compound
    BOOST_LOG_API static stream_compound* allocate_compound(record const& rec);
    //! The method releases a compound
    BOOST_LOG_API static void release_compound(stream_compound* compound) /* throw() */;

    //  Non-constructible, non-copyable, non-assignable
    BOOST_LOG_DELETED_FUNCTION(stream_provider())
    BOOST_LOG_DELETED_FUNCTION(stream_provider(stream_provider const&))
    BOOST_LOG_DELETED_FUNCTION(stream_provider& operator= (stream_provider const&))
};


/*!
 * \brief Logging record pump implementation
 *
 * The pump is used to format the logging record message text and then
 * push it to the logging core. It is constructed on each attempt to write
 * a log record and destroyed afterwards.
 *
 * The pump class template is instantiated on the logger type.
 */
template< typename LoggerT >
class record_pump
{
private:
    //! Logger type
    typedef LoggerT logger_type;
    //! Character type
    typedef typename logger_type::char_type char_type;
    //! Stream compound provider
    typedef stream_provider< char_type > stream_provider_type;
    //! Stream compound type
    typedef typename stream_provider_type::stream_compound stream_compound;

    //! Stream compound release guard
    class auto_release;
    friend class auto_release;
    class auto_release
    {
        stream_compound* m_pCompound;

    public:
        explicit auto_release(stream_compound* p) : m_pCompound(p) {}
        ~auto_release() { stream_provider_type::release_compound(m_pCompound); }
    };

protected:
    //! A reference to the logger
    mutable logger_type* m_pLogger;
    //! Stream compound
    mutable stream_compound* m_pStreamCompound;

public:
    //! Constructor
    explicit record_pump(logger_type& lg, record const& rec) :
        m_pLogger(boost::addressof(lg)),
        m_pStreamCompound(stream_provider_type::allocate_compound(rec))
    {
    }
    //! Copy constructor (implemented as move)
    record_pump(record_pump const& that) :
        m_pLogger(that.m_pLogger),
        m_pStreamCompound(that.m_pStreamCompound)
    {
        that.m_pLogger = 0;
        that.m_pStreamCompound = 0;
    }
    //! Destructor. Pushes the composed message to log.
    ~record_pump()
    {
        if (m_pLogger)
        {
            auto_release cleanup(m_pStreamCompound); // destructor doesn't throw
            if (!std::uncaught_exception())
                m_pLogger->push_record(m_pStreamCompound->stream.get_record());
        }
    }

    //! Returns the stream to be used for message text formatting
    basic_record_ostream< char_type >& stream() const
    {
        BOOST_ASSERT(m_pStreamCompound != 0);
        return m_pStreamCompound->stream;
    }

    //! Closed assignment
    BOOST_LOG_DELETED_FUNCTION(record_pump& operator= (record_pump const&))
};

template< typename LoggerT >
BOOST_LOG_FORCEINLINE record_pump< LoggerT > make_pump_stream(LoggerT& lg, record const& rec)
{
    return record_pump< LoggerT >(lg, rec);
}

} // namespace aux

//! \cond

#define BOOST_LOG_STREAM_INTERNAL(logger, rec_var)\
    for (::boost::log::record rec_var = (logger).open_record(); !!rec_var; rec_var.reset())\
        ::boost::log::aux::make_pump_stream((logger), rec_var).stream()

#define BOOST_LOG_STREAM_WITH_PARAMS_INTERNAL(logger, rec_var, params_seq)\
    for (::boost::log::record rec_var = (logger).open_record((BOOST_PP_SEQ_ENUM(params_seq))); !!rec_var; rec_var.reset())\
        ::boost::log::aux::make_pump_stream((logger), rec_var).stream()

//! \endcond

//! The macro writes a record to the log
#define BOOST_LOG_STREAM(logger)\
    BOOST_LOG_STREAM_INTERNAL(logger, BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_boost_log_record_))

//! The macro writes a record to the log and allows to pass additional named arguments to the logger
#define BOOST_LOG_STREAM_WITH_PARAMS(logger, params_seq)\
    BOOST_LOG_STREAM_WITH_PARAMS_INTERNAL(logger, BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_boost_log_record_), params_seq)

#ifndef BOOST_LOG_NO_SHORTHAND_NAMES

//! An equivalent to BOOST_LOG_STREAM(logger)
#define BOOST_LOG(logger) BOOST_LOG_STREAM(logger)

//! An equivalent to BOOST_LOG_STREAM_WITH_PARAMS(logger, params_seq)
#define BOOST_LOG_WITH_PARAMS(logger, params_seq) BOOST_LOG_STREAM_WITH_PARAMS(logger, params_seq)

#endif // BOOST_LOG_NO_SHORTHAND_NAMES

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SOURCES_RECORD_OSTREAM_HPP_INCLUDED_
