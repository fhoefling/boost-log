/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   attr.hpp
 * \author Andrey Semashev
 * \date   21.07.2012
 *
 * The header contains implementation of a generic attribute placeholder in template expressions.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_

#include <boost/parameter/binding.hpp>
#include <boost/phoenix/core/actor.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/parameter_tools.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/attributes/fallback_policy.hpp>
#include <boost/log/keywords/attr_tag.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

namespace aux {

template< typename BaseT >
class extractor_adapter :
    public BaseT
{
    //! Base function type
    typedef BaseT base_type;
    //! Self type
    typedef extractor_adapter< base_type > this_type;

private:
    //! Attribute value name
    const attribute_name m_name;

public:
    //! Function result type
    template< typename >
    struct result;

    template< typename ArgsT >
    struct result< this_type(ArgsT) > :
        public boost::result_of< base_type(attribute_name, typename fusion::result_of::at_c< ArgsT, 0 >::type) >
    {
    };

    template< typename ArgsT >
    struct result< const this_type(ArgsT) > :
        public boost::result_of< const base_type(attribute_name, typename fusion::result_of::at_c< ArgsT, 0 >::type) >
    {
    };

public:
    //! Default constructor
    explicit extractor_adapter(attribute_name const& name) : m_name(name)
    {
    }
    //! Initializing constructor
    template< typename ArgT1 >
    extractor_adapter(attribute_name const& name, ArgT1 const& arg1) : base_type(arg1), m_name(name)
    {
    }
    //! Initializing constructor
    template< typename ArgT1, typename ArgT2 >
    extractor_adapter(attribute_name const& name, ArgT1 const& arg1, ArgT2 const& arg2) : base_type(arg1, arg2), m_name(name)
    {
    }

    //! Returns attribute value name
    attribute_name const& get_name() const
    {
        return m_name;
    }

    //! The operator forwards the call to the base function
    template< typename ArgsT >
    typename boost::result_of< base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >::type
    operator() (ArgsT const& args)
    {
        return base_type::operator()(m_name, fusion::at_c< 0 >(args));
    }

    //! The operator forwards the call to the base function
    template< typename ArgsT >
    typename boost::result_of< const base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >::type
    operator() (ArgsT const& args) const
    {
        return base_type::operator()(m_name, fusion::at_c< 0 >(args));
    }
};

} // namespace aux

/*!
 * An attribute value extraction terminal
 */
template<
    typename T,
    typename FallbackPolicyT = fallback_to_none,
    typename ParamsT = boost::log::aux::empty_arg_list,
    template< typename > class ActorT = phoenix::actor
>
class attribute_terminal :
    public ActorT<
        terminal<
            aux::extractor_adapter<
                value_extractor<
                    T,
                    FallbackPolicyT,
                    typename remove_cv< typename parameter::binding< ParamsT, keywords::tag::attr_tag, void >::type >::type
                >
            >
        >
    >
{
public:
    //! Attribute value extractor
    typedef value_extractor<
        T,
        FallbackPolicyT,
        typename remove_cv< typename parameter::binding< ParamsT, keywords::tag::attr_tag, void >::type >::type
    > extractor_type;
    //! Base terminal type
    typedef terminal< aux::extractor_adapter< extractor_type > > terminal_type;
    //! Base actor type
    typedef ActorT< terminal_type > base_type;
    //! Attribute value type
    typedef typename extractor_type::value_type value_type;

    //! Additional parameters
    typedef ParamsT parameters_type;

private:
    //! Additional parameters
    parameters_type m_params;

public:
    //! Initializing constructor
    explicit attribute_terminal(base_type const& act, parameters_type const& params = parameters_type()) : base_type(act), m_params(params)
    {
    }

    //! Returns the attribute name
    attribute_name get_name() const
    {
        return this->proto_expr_.get_name();
    }

    //! Returns additional parameters
    parameters_type const& get_params() const
    {
        return m_params;
    }

    //! Expression with cached attribute name
    typedef attribute_terminal< value_type, fallback_to_none, parameters_type, ActorT > or_none_result_type;

    //! Generates an expression that extracts the attribute value or a default value
    or_none_result_type or_none() const
    {
        typedef typename or_none_result_type::terminal_type result_terminal;
        base_type act = { result_terminal(this->proto_expr_.get_name()) };
        return or_none_result_type(act);
    }

    //! Expression with cached attribute name
    typedef attribute_terminal< value_type, fallback_to_throw, parameters_type, ActorT > or_throw_result_type;

    //! Generates an expression that extracts the attribute value or throws an exception
    or_throw_result_type or_throw() const
    {
        typedef typename or_throw_result_type::terminal_type result_terminal;
        base_type act = { result_terminal(this->proto_expr_.get_name()) };
        return or_throw_result_type(act);
    }

    //! Generates an expression that extracts the attribute value or a default value
    template< typename DefaultT >
    attribute_terminal< value_type, fallback_to_default< DefaultT >, parameters_type, ActorT > or_default(DefaultT const& def_val) const
    {
        typedef attribute_terminal< value_type, fallback_to_default< DefaultT >, parameters_type, ActorT > or_default_result_type;
        typedef typename or_default_result_type::terminal_type result_terminal;
        base_type act = { result_terminal(get_name(), def_val) };
        return or_default_result_type(act);
    }
};

/*!
 * The function generates a terminal node in a template expression. The node will extract the value of the attribute
 * with the specified name and type.
 */
template< typename AttributeValueT >
BOOST_LOG_FORCEINLINE attribute_terminal< AttributeValueT > attr(attribute_name const& name)
{
    typedef attribute_terminal< AttributeValueT > result_type;
    typedef typename result_type::terminal_type result_terminal;
    typename result_type::base_type act = { result_terminal(name) };
    return result_type(act);
}

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_
