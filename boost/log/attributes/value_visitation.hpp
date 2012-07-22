/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   value_visitation.hpp
 * \author Andrey Semashev
 * \date   01.03.2008
 *
 * The header contains implementation of convenience tools to apply visitors to an attribute value
 * in the view.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_VALUE_VISITATION_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_VALUE_VISITATION_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * \brief The class represents attribute value visitation result
 *
 * The main purpose of this class is to provide a convenient interface for checking
 * whether the attribute value visitation succeeded or not. It also allows to discover
 * the actual cause of failure, should the operation fail.
 */
class visitation_result
{
public:
    //! Error codes for attribute value visitation
    enum error_code
    {
        ok,                     //!< The attribute value has been visited successfully
        value_not_found,        //!< The attribute value is not present in the view
        value_has_invalid_type  //!< The attribute value is present in the vew, but has an unexpected type
    };

private:
    error_code m_Code;

public:
    /*!
     * Initializing constructor. Creates the result that is equivalent to the
     * specified error code.
     */
    visitation_result(error_code code = ok) : m_Code(code) {}

    /*!
     * Checks if the visitation was successful.
     *
     * \return \c true if the value was visited successfully, \c false otherwise.
     */
    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()
    /*!
     * Checks if the visitation was unsuccessful.
     *
     * \return \c false if the value was visited successfully, \c true otherwise.
     */
    bool operator! () const { return (m_Code != ok); }

    /*!
     * \return The actual result code of value visitation
     */
    error_code code() const { return m_Code; }
};

/*!
 * \brief Generic attribute value visitor invoker
 *
 * Attribute value invoker is a functional object that attempts to find and extract the stored
 * attribute value from the attribute value view or a log record. The extracted value is passed to
 * an unary function object (the visitor) provided by user.
 *
 * The invoker can be specialized on one or several attribute value types that should be
 * specified in the second template argument.
 */
template< typename T >
class value_visitor_invoker
{
public:
    //! Function object result type
    typedef visitation_result result_type;

    //! Attribute value types
    typedef T value_types;

private:
    //! The name of the attribute value to visit
    attribute_name m_Name;

public:
    /*!
     * Constructor
     *
     * \param name Attribute name to be visited on invokation
     */
    explicit value_visitor_invoker(attribute_name const& name) : m_Name(name) {}

    /*!
     * Visitation operator. Looks for an attribute value with the name specified on construction
     * and tries to acquire the stored value of one of the supported types. If acquisition succeeds,
     * the value is passed to \a visitor.
     *
     * \param attrs A set of attribute values in which to look for the specified attribute value.
     * \param visitor A receiving function object to pass the attribute value to.
     * \return The result of visitation (see codes in the \c visitation_result class).
     */
    template< typename VisitorT >
    result_type operator() (attribute_values_view const& attrs, VisitorT visitor) const
    {
        try
        {
            attribute_values_view::const_iterator it = attrs.find(m_Name);
            if (it != attrs.end())
            {
                if (it->second.visit< value_types >(visitor))
                    return visitation_result::ok;
                else
                    return visitation_result::value_has_invalid_type;
            }
            return visitation_result::value_not_found;
        }
        catch (exception& e)
        {
            // Attach the attribute name to the exception
            boost::log::aux::attach_attribute_name_info(e, m_Name);
            throw;
        }
    }

    /*!
     * Visitation operator. Looks for an attribute value with the name specified on construction
     * and tries to acquire the stored value of one of the supported types. If acquisition succeeds,
     * the value is passed to \a visitor.
     *
     * \param record A log record. The attribute value will be sought among those associated with the record.
     * \param visitor A receiving function object to pass the attribute value to.
     * \return The result of visitation (see codes in the \c visitation_result class).
     */
    template< typename VisitorT >
    result_type operator() (record const& rec, VisitorT visitor) const
    {
        return operator() (rec.attribute_values(), visitor);
    }
};

#ifdef BOOST_LOG_DOXYGEN_PASS

/*!
 * The function applies a visitor to an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \param name The name of the attribute value to visit.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \param visitor A receiving function object to pass the attribute value to.
 * \return The result of visitation (see codes in the \c visitation_result class).
 */
template< typename T, typename VisitorT >
visitation_result visit(attribute_name const& name, attribute_values_view const& attrs, VisitorT visitor);

/*!
 * The function applies a visitor to an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \param name The name of the attribute value to visit.
 * \param rec A log record. The attribute value will be sought among those associated with the record.
 * \param visitor A receiving function object to pass the attribute value to.
 * \return The result of visitation (see codes in the \c visitation_result class).
 */
template< typename T, typename VisitorT >
visitation_result visit(attribute_name const& name, record const& rec, VisitorT visitor);

#else // BOOST_LOG_DOXYGEN_PASS

template< typename T, typename VisitorT >
inline visitation_result visit(attribute_name const& name, attribute_values_view const& attrs, VisitorT visitor)
{
    value_visitor_invoker< T > invoker(name);
    return invoker(attrs, visitor);
}

template< typename T, typename VisitorT >
inline visitation_result visit(attribute_name const& name, record const& rec, VisitorT visitor)
{
    value_visitor_invoker< T > invoker(name);
    return invoker(rec, visitor);
}

#endif // BOOST_LOG_DOXYGEN_PASS

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_VALUE_VISITATION_HPP_INCLUDED_
