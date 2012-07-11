/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   event_log_backend.hpp
 * \author Andrey Semashev
 * \date   07.11.2008
 *
 * The header contains a logging sink backend that uses Windows NT event log API
 * for signaling application events.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_EVENT_LOG_BACKEND_HPP_INCLUDED_
#define BOOST_LOG_SINKS_EVENT_LOG_BACKEND_HPP_INCLUDED_

#include <map>
#include <vector>
#include <string>
#include <iosfwd>
#include <boost/filesystem/path.hpp>
#include <boost/function/function1.hpp>
#include <boost/function/function3.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/universal_path.hpp>
#include <boost/log/detail/parameter_tools.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/keywords/message_file.hpp>
#include <boost/log/keywords/log_name.hpp>
#include <boost/log/keywords/log_source.hpp>
#include <boost/log/keywords/registration.hpp>
#include <boost/log/keywords/target.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/attribute_mapping.hpp>
#include <boost/log/sinks/event_log_constants.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace event_log {

    //! Event log source registration modes
    enum registration_mode
    {
        never,      //!< Never register event source, even if it's not registered
        on_demand,  //!< Register if the source is not registered yet
        forced      //!< Register always, event if the source is already registered
    };

    /*!
     * \brief Straightforward event type mapping
     *
     * This type of mapping assumes that attribute with a particular name always
     * provides values that map directly onto the native event types. The mapping
     * simply returns the extracted attribute value converted to the native event type.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_direct_event_type_mapping :
        public basic_direct_mapping< CharT, event_type_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_mapping< CharT, event_type_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit basic_direct_event_type_mapping(string_type const& name) :
            base_type(name, info)
        {
        }
    };

    /*!
     * \brief Customizable event type mapping
     *
     * The class allows to setup a custom mapping between an attribute and native event types.
     * The mapping should be initialized similarly to the standard \c map container, by using
     * indexing operator and assignment.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_custom_event_type_mapping :
        public basic_custom_mapping< CharT, event_type_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_mapping< CharT, event_type_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit basic_custom_event_type_mapping(string_type const& name) :
            base_type(name, info)
        {
        }
    };

    /*!
     * \brief Straightforward event ID mapping
     *
     * This type of mapping assumes that attribute with a particular name always
     * provides values that map directly onto the event identifiers. The mapping
     * simply returns the extracted attribute value converted to the event ID.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_direct_event_id_mapping :
        public basic_direct_mapping< CharT, event_id_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_mapping< CharT, event_id_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit basic_direct_event_id_mapping(string_type const& name) :
            base_type(name, make_event_id(0))
        {
        }
    };

    /*!
     * \brief Customizable event ID mapping
     *
     * The class allows to setup a custom mapping between an attribute and event identifiers.
     * The mapping should be initialized similarly to the standard \c map container, by using
     * indexing operator and assignment.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_custom_event_id_mapping :
        public basic_custom_mapping< CharT, event_id_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_mapping< CharT, event_id_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit basic_custom_event_id_mapping(string_type const& name) :
            base_type(name, make_event_id(0))
        {
        }
    };

    /*!
     * \brief Straightforward event category mapping
     *
     * This type of mapping assumes that attribute with a particular name always
     * provides values that map directly onto the event categories. The mapping
     * simply returns the extracted attribute value converted to the event category.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_direct_event_category_mapping :
        public basic_direct_mapping< CharT, event_category_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_mapping< CharT, event_category_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit basic_direct_event_category_mapping(string_type const& name) :
            base_type(name, make_event_category(0))
        {
        }
    };

    /*!
     * \brief Customizable event category mapping
     *
     * The class allows to setup a custom mapping between an attribute and event categories.
     * The mapping should be initialized similarly to the standard \c map container, by using
     * indexing operator and assignment.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_custom_event_category_mapping :
        public basic_custom_mapping< CharT, event_category_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_mapping< CharT, event_category_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit basic_custom_event_category_mapping(string_type const& name) :
            base_type(name, make_event_category(0))
        {
        }
    };

#ifdef BOOST_LOG_USE_CHAR

    /*!
     * \brief Straightforward event type mapping
     *
     * This is a convenience template typedef over \c basic_direct_event_type_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class direct_event_type_mapping :
        public basic_direct_event_type_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_event_type_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit direct_event_type_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable event type mapping
     *
     * This is a convenience template typedef over \c basic_custom_event_type_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class custom_event_type_mapping :
        public basic_custom_event_type_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_event_type_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit custom_event_type_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Straightforward event id mapping
     *
     * This is a convenience template typedef over \c basic_direct_event_id_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class direct_event_id_mapping :
        public basic_direct_event_id_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_event_id_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit direct_event_id_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable event id mapping
     *
     * This is a convenience template typedef over \c basic_custom_event_id_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class custom_event_id_mapping :
        public basic_custom_event_id_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_event_id_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit custom_event_id_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Straightforward event category mapping
     *
     * This is a convenience template typedef over \c basic_direct_event_category_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class direct_event_category_mapping :
        public basic_direct_event_category_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_event_category_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit direct_event_category_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable event category mapping
     *
     * This is a convenience template typedef over \c basic_custom_event_category_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class custom_event_category_mapping :
        public basic_custom_event_category_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_event_category_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit custom_event_category_mapping(string_type const& name) : base_type(name)
        {
        }
    };

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

    /*!
     * \brief Straightforward event type mapping
     *
     * This is a convenience template typedef over \c basic_direct_event_type_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wdirect_event_type_mapping :
        public basic_direct_event_type_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_event_type_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit wdirect_event_type_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable event type mapping
     *
     * This is a convenience template typedef over \c basic_custom_event_type_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wcustom_event_type_mapping :
        public basic_custom_event_type_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_event_type_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit wcustom_event_type_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Straightforward event id mapping
     *
     * This is a convenience template typedef over \c basic_direct_event_id_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wdirect_event_id_mapping :
        public basic_direct_event_id_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_event_id_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit wdirect_event_id_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable event id mapping
     *
     * This is a convenience template typedef over \c basic_custom_event_id_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wcustom_event_id_mapping :
        public basic_custom_event_id_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_event_id_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit wcustom_event_id_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Straightforward event category mapping
     *
     * This is a convenience template typedef over \c basic_direct_event_category_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wdirect_event_category_mapping :
        public basic_direct_event_category_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_event_category_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit wdirect_event_category_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable event category mapping
     *
     * This is a convenience template typedef over \c basic_custom_event_category_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wcustom_event_category_mapping :
        public basic_custom_event_category_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_event_category_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         *
         * \param name Attribute name
         */
        explicit wcustom_event_category_mapping(string_type const& name) : base_type(name)
        {
        }
    };

#endif // BOOST_LOG_USE_WCHAR_T


    /*!
     * \brief An event composer
     *
     * This class is a function object that extracts event identifier from the attribute values set
     * and formats insertion strings for the particular event. Each insertion string is formatted with
     * a distinct formatter, which can be created just like regular sinks formatters.
     *
     * Before using, the composer must be initialized with the following information:
     * \li Event identifier extraction logic. One can use \c basic_direct_event_id_mapping or
     *     \c basic_custom_event_id_mapping classes in order to create such extractor and pass it
     *     to the composer constructor.
     * \li Event identifiers and insertion string formatters. The composer provides the following
     *     syntax to provide this information:
     *
     *     \code
     *     event_composer comp;
     *     comp[MY_EVENT_ID1] % formatter1 % ... % formatterN;
     *     comp[MY_EVENT_ID2] % formatter1 % ... % formatterN;
     *     ...
     *     \endcode
     *
     *     The event identifiers in square brackets are provided by the message compiler generated
     *     header (the actual names are specified in the .mc file). The formatters represent
     *     the insertion strings that will be used to replace placeholders in event messages,
     *     thus the number and the order of the formatters must correspond to the message definition.
     */
    template< typename CharT >
    class BOOST_LOG_EXPORT basic_event_composer
    {
    public:
        //! Character type
        typedef CharT char_type;
        //! String type to be used as a message text holder
        typedef std::basic_string< char_type > string_type;
        //! Output stream type
        typedef std::basic_ostream< char_type > stream_type;
        //! Attribute values view type
        typedef basic_attribute_values_view< char_type > values_view_type;
        //! Log record type
        typedef basic_record< char_type > record_type;

        //! Event identifier mapper type
        typedef function1< event_id_t, values_view_type const& > event_id_mapper_type;

        //! Type of an insertion composer (a formatter)
        typedef function2<
            void,
            stream_type&,
            record_type const&
        > formatter_type;
        //! Type of the composed insertions list
        typedef std::vector< string_type > insertion_list;

    private:
        //! \cond

        //! The class that implements formatting of insertion strings
        class insertion_composer;

        //! Type of the events map
        typedef std::map< event_id_t, insertion_composer > event_map;

        //! A smart reference that puts formatters into the composer
        class event_map_reference;
        friend class event_map_reference;
        class event_map_reference
        {
        private:
            //! Event identifier
            event_id_t m_ID;
            //! A reference to the object that created the reference
            basic_event_composer< char_type >& m_Owner;
            //! A hint for the owner to optimize insertion
            insertion_composer* m_Composer;

        public:
            //! Initializing constructor
            explicit event_map_reference(event_id_t id, basic_event_composer< char_type >& owner) :
                m_ID(id),
                m_Owner(owner),
                m_Composer(0)
            {
            }
            //! The operator puts the formatter into the composer
            template< typename FormatterT >
            event_map_reference& operator% (FormatterT const& fmt)
            {
                m_Composer = m_Owner.add_formatter(m_ID, m_Composer, formatter_type(fmt));
            }
        };

        //! \endcond

    private:
        //! The mapper that will extract the event identifier
        event_id_mapper_type m_EventIDMapper;
        //! The map of event identifiers and and their insertion composers
        event_map m_EventMap;

    public:
        /*!
         * Default constructor. Creates an empty map of events.
         *
         * \param id_mapper An event identifier mapping function that will be used to extract event ID from attribute values
         */
        explicit basic_event_composer(event_id_mapper_type const& id_mapper);
        /*!
         * Copy constructor. Performs a deep copy of the object.
         */
        basic_event_composer(basic_event_composer const& that);
        /*!
         * Destructor
         */
        ~basic_event_composer();

        /*!
         * Assignment. Provides strong exception guarantee.
         */
        basic_event_composer& operator= (basic_event_composer that);
        /*!
         * Swaps \c *this and \c that objects.
         */
        void swap(basic_event_composer& that);
        /*!
         * Initiates creation of a new event description. The result of the operator can be used to
         * add formatters for insertion strings construction. The returned reference type is implementation detail.
         *
         * \param id Event identifier.
         */
        event_map_reference operator[] (event_id_t id);
        /*!
         * Initiates creation of a new event description. The result of the operator can be used to
         * add formatters for insertion strings construction. The returned reference type is implementation detail.
         *
         * \param id Event identifier.
         */
        event_map_reference operator[] (event_id_t::integer_type id);
        /*!
         * Event composition operator. Extracts an event identifier from the attribute values by calling event ID mapper.
         * Then runs all formatters that were registered for the event with the extracted ID. The results of formatting
         * are returned in the \c insertions parameter.
         *
         * \param rec Log record
         * \param insertions A sequence of formatted insertion strings
         * \return An event identifier that was extracted from \c attributes
         */
        event_id_t operator() (record_type const& rec, insertion_list& insertions) const;

    private:
#ifndef BOOST_LOG_DOXYGEN_PASS
        //! Adds a formatter to the insertion composers list
        insertion_composer* add_formatter(event_id_t id, insertion_composer* composer, formatter_type const& fmt);
#endif // BOOST_LOG_DOXYGEN_PASS
    };

#ifdef BOOST_LOG_USE_CHAR
    typedef basic_event_composer< char > event_composer;          //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
    typedef basic_event_composer< wchar_t > wevent_composer;      //!< Convenience typedef for wide-character logging
#endif

} // namespace event_log

/*!
 * \brief An implementation of a simple logging sink backend that emits events into Windows NT event log
 *
 * The sink uses Windows NT 5 (Windows 2000) and later event log API to emit events
 * to an event log. The sink acts as an event source in terms of the API, it implements all needed resources
 * and source registration in the Windows registry that is needed for the event delivery.
 *
 * The backend performs message text formatting. The composed text is then passed as the first
 * and only string parameter of the event. The resource embedded into the backend describes the event
 * so that the parameter is inserted into the event description text, thus making it visible
 * in the event log.
 *
 * The backend allows to customize mapping of application severity levels to the native Windows event types.
 * This allows to write portable code even if OS-specific sinks, such as this one, are used.
 *
 * \note Since the backend registers itself into Windows registry as the resource file that contains
 *       event description, it is important to keep the library binary in a stable place of the filesystem.
 *       Otherwise Windows might not be able to load event resources from the library and display
 *       events correctly.
 *
 * \note It is known that Windows is not able to find event resources in the application executable,
 *       which is linked against the static build of the library. Users are advised to use dynamic
 *       builds of the library to solve this problem.
 */
template< typename CharT >
class basic_simple_event_log_backend :
    public basic_formatting_sink_backend< CharT >
{
    //! Base type
    typedef basic_formatting_sink_backend< CharT > base_type;
    //! Implementation type
    struct implementation;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! String type to be used as a message text holder
    typedef typename base_type::target_string_type target_string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;
    //! Log record type
    typedef typename base_type::record_type record_type;

    //! Mapper type for the event type
    typedef function1<
        event_log::event_type_t,
        values_view_type const&
    > event_type_mapper_type;

private:
    //! Pointer to the backend implementation that hides various types from windows.h
    implementation* m_pImpl;

public:
    /*!
     * Default constructor. Registers event source with name based on the application
     * executable file name in the Application log. If such a registration is already
     * present, it is not overridden.
     */
    BOOST_LOG_EXPORT basic_simple_event_log_backend();
    /*!
     * Constructor. Registers event log source with the specified parameters.
     * The following named parameters are supported:
     *
     * \li \c target - Specifies an UNC path to the remote server which log records should be sent to.
     *     The local machine will be used to process log records, if not specified.
     * \li \c log_name - Specifies the log in which the source should be registered.
     *     The result of \c get_default_log_name is used, if the parameter is not specified.
     * \li \c log_source - Specifies the source name. The result of \c get_default_source_name
     *     is used, if the parameter is not specified.
     * \li \c registration - Specifies the event source registration mode in the Windows registry.
     *     Can have values of the \c registration_mode enum. Default value: \c on_demand.
     *
     * \param args A set of named parameters.
     */
#ifndef BOOST_LOG_DOXYGEN_PASS
    BOOST_LOG_PARAMETRIZED_CONSTRUCTORS_CALL(basic_simple_event_log_backend, construct)
#else
    template< typename... ArgsT >
    explicit basic_simple_event_log_backend(ArgsT... const& args);
#endif

    /*!
     * Destructor. Unregisters event source. The log source description is not removed from the Windows registry.
     */
    BOOST_LOG_EXPORT ~basic_simple_event_log_backend();

    /*!
     * The method installs the function object that maps application severity levels to WinAPI event types
     */
    BOOST_LOG_EXPORT void set_event_type_mapper(event_type_mapper_type const& mapper);

    /*!
     * \returns Default log name: Application
     */
    BOOST_LOG_EXPORT static string_type get_default_log_name();
    /*!
     * \returns Default log source name that is based on the application executable file name and the sink name
     */
    BOOST_LOG_EXPORT static string_type get_default_source_name();

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! The method puts the formatted message to the event log
    BOOST_LOG_EXPORT void do_consume(record_type const& record, target_string_type const& formatted_message);

    //! Constructs backend implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
            args[keywords::target | string_type()],
            args[keywords::log_name || &basic_simple_event_log_backend::get_default_log_name],
            args[keywords::log_source || &basic_simple_event_log_backend::get_default_source_name],
            args[keywords::registration | event_log::on_demand]);
    }
    BOOST_LOG_EXPORT void construct(
        string_type const& target,
        string_type const& log_name,
        string_type const& source_name,
        event_log::registration_mode reg_mode);
#endif // BOOST_LOG_DOXYGEN_PASS
};

/*!
 * \brief An implementation of a logging sink backend that emits events into Windows NT event log
 *
 * The sink uses Windows NT 5 (Windows 2000) and later event log API to emit events
 * to an event log. The sink acts as an event source. Unlike \c basic_simple_event_log_backend,
 * this sink backend allows users to specify the custom event message file and supports
 * mapping attribute values onto several insertion strings. Although it requires considerably
 * more scaffolding than the simple backend, this allows to support localizable event descriptions.
 *
 * Besides the file name of the module with event resources, the backend provides the following
 * customizations:
 * \li Remote server UNC address, log name and source name. These parameters have similar meaning
 *     to \c basic_simple_event_log_backend.
 * \li Event type and category mappings. These are function object that allow to map attribute
 *     values to the according event parameters. One can use mappings in the \c event_log namespace.
 * \li Event composer. This function object extracts event identifier and formats string insertions,
 *     that will be used by the API to compose the final event message text.
 */
template< typename CharT >
class basic_event_log_backend :
    public basic_sink_backend< CharT, frontend_synchronization_tag >
{
    //! Base type
    typedef basic_sink_backend< CharT, frontend_synchronization_tag > base_type;
    //! Implementation type
    struct implementation;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;
    //! Log record type
    typedef typename base_type::record_type record_type;
    //! Type of the composed insertions list
    typedef std::vector< string_type > insertion_list;

    //! Mapper type for the event type
    typedef function1<
        event_log::event_type_t,
        values_view_type const&
    > event_type_mapper_type;
    //! Mapper type for the event category
    typedef function1<
        event_log::event_category_t,
        values_view_type const&
    > event_category_mapper_type;
    //! Event composer type
    typedef function3<
        event_log::event_id_t,
        record_type const&,
        insertion_list&
    > event_composer_type;

private:
    //! Pointer to the backend implementation that hides various types from windows.h
    implementation* m_pImpl;

public:
    /*!
     * Constructor. Registers event source with name based on the application
     * executable file name in the Application log. If such a registration is already
     * present, it is not overridden.
     */
    template< typename T >
    explicit basic_event_log_backend(std::basic_string< T > const& message_file_name)
    {
        construct(keywords::message_file = message_file_name);
    }
    /*!
     * Constructor. Registers event source with name based on the application
     * executable file name in the Application log. If such a registration is already
     * present, it is not overridden.
     */
#if (defined(BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION >= 3) || defined(BOOST_LOG_DOXYGEN_PASS)
    explicit basic_event_log_backend(filesystem::path const& message_file_name)
#else
    template< typename T, typename U >
    explicit basic_event_log_backend(filesystem::basic_path< T, U > const& message_file_name)
#endif
    {
        construct(keywords::message_file = message_file_name);
    }
    /*!
     * Constructor. Registers event log source with the specified parameters.
     * The following named parameters are supported:
     *
     * \li \c message_file - Specifies the file name that contains resources that
     *     describe events and categories.
     * \li \c target - Specifies an UNC path to the remote server to which log records should be sent to.
     *     The local machine will be used to process log records, if not specified.
     * \li \c log_name - Specifies the log in which the source should be registered.
     *     The result of \c get_default_log_name is used, if the parameter is not specified.
     * \li \c log_source - Specifies the source name. The result of \c get_default_source_name
     *     is used, if the parameter is not specified.
     * \li \c registration - Specifies the event source registration mode in the Windows registry.
     *     Can have values of the \c registration_mode enum. Default value: \c on_demand.
     *
     * \param args A set of named parameters.
     */
#ifndef BOOST_LOG_DOXYGEN_PASS
    BOOST_LOG_PARAMETRIZED_CONSTRUCTORS_CALL(basic_event_log_backend, construct)
#else
    template< typename... ArgsT >
    explicit basic_event_log_backend(ArgsT... const& args);
#endif

    /*!
     * Destructor. Unregisters event source. The log source description is not removed from the Windows registry.
     */
    BOOST_LOG_EXPORT ~basic_event_log_backend();

    /*!
     * The method creates an event in the event log
     *
     * \param record Log record to consume
     */
    BOOST_LOG_EXPORT void consume(record_type const& record);

    /*!
     * The method installs the function object that maps application severity levels to WinAPI event types
     */
    BOOST_LOG_EXPORT void set_event_type_mapper(event_type_mapper_type const& mapper);

    /*!
     * The method installs the function object that extracts event category from attribute values
     */
    BOOST_LOG_EXPORT void set_event_category_mapper(event_category_mapper_type const& mapper);

    /*!
     * The method installs the function object that extracts event identifier from the attributes and creates
     * insertion strings that will replace placeholders in the event message.
     */
    BOOST_LOG_EXPORT void set_event_composer(event_composer_type const& composer);

    /*!
     * \returns Default log name: Application
     */
    BOOST_LOG_EXPORT static string_type get_default_log_name();
    /*!
     * \returns Default log source name that is based on the application executable file name and the sink name
     */
    BOOST_LOG_EXPORT static string_type get_default_source_name();

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! Constructs backend implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
            boost::log::aux::to_universal_path(args[keywords::message_file]),
            args[keywords::target | string_type()],
            args[keywords::log_name || &basic_event_log_backend::get_default_log_name],
            args[keywords::log_source || &basic_event_log_backend::get_default_source_name],
            args[keywords::registration | event_log::on_demand]);
    }
    BOOST_LOG_EXPORT void construct(
        boost::log::aux::universal_path const& message_file_name,
        string_type const& target,
        string_type const& log_name,
        string_type const& source_name,
        event_log::registration_mode reg_mode);
#endif // BOOST_LOG_DOXYGEN_PASS
};

#ifdef BOOST_LOG_USE_CHAR
typedef basic_simple_event_log_backend< char > simple_event_log_backend;      //!< Convenience typedef for narrow-character logging
typedef basic_event_log_backend< char > event_log_backend;                    //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_simple_event_log_backend< wchar_t > wsimple_event_log_backend;  //!< Convenience typedef for wide-character logging
typedef basic_event_log_backend< wchar_t > wevent_log_backend;                //!< Convenience typedef for wide-character logging
#endif

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_EVENT_LOG_BACKEND_HPP_INCLUDED_
