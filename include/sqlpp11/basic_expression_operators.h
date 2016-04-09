/*
 * Copyright (c) 2013-2016, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SQLPP_DETAIL_BASIC_EXPRESSION_OPERATORS_H
#define SQLPP_DETAIL_BASIC_EXPRESSION_OPERATORS_H

#include <sqlpp11/value_type_fwd.h>
#include <sqlpp11/bad_statement.h>
#include <sqlpp11/portable_static_assert.h>
#include <sqlpp11/consistent.h>
#include <sqlpp11/alias.h>
#include <sqlpp11/sort_order.h>
#include <sqlpp11/expression_fwd.h>
#include <sqlpp11/in_fwd.h>
#include <sqlpp11/is_null_fwd.h>
#include <sqlpp11/wrap_operand.h>
#include <sqlpp11/logic.h>

namespace sqlpp
{
  SQLPP_PORTABLE_STATIC_ASSERT(assert_comparison_rhs_is_expression_t, "rhs operand in comparison is not an expression");
  SQLPP_PORTABLE_STATIC_ASSERT(assert_comparison_rhs_is_valid_operand_t, "invalid rhs operand in comparison");
  SQLPP_PORTABLE_STATIC_ASSERT(assert_comparison_lhs_rhs_differ_t, "identical lhs and rhs operands in comparison");

  template <typename LhsType, typename RhsType>
  using check_comparison_t = static_combined_check_t<
      static_check_t<logic::any_t<is_expression_t<sqlpp::wrap_operand_t<RhsType>>::value,
                                  is_multi_expression_t<sqlpp::wrap_operand_t<RhsType>>::value>::value,
                     assert_comparison_rhs_is_expression_t>,
      static_check_t<value_type_of<LhsType>::template _is_valid_operand<sqlpp::wrap_operand_t<RhsType>>::value,
                     assert_comparison_rhs_is_valid_operand_t>,
      static_check_t<not std::is_same<LhsType, RhsType>::value, assert_comparison_lhs_rhs_differ_t>>;

  template <typename LhsType, typename... RhsType>
  using check_in_t = static_combined_check_t<
      static_check_t<logic::all_t<is_expression_t<sqlpp::wrap_operand_t<RhsType>>::value...>::value,
                     assert_comparison_rhs_is_expression_t>,
      static_check_t<logic::all_t<value_type_of<LhsType>::template _is_valid_operand<
                         sqlpp::wrap_operand_t<RhsType>>::value...>::value,
                     assert_comparison_rhs_is_valid_operand_t>,
      static_check_t<logic::none_t<std::is_same<LhsType, RhsType>::value...>::value,
                     assert_comparison_lhs_rhs_differ_t>>;

  namespace detail
  {
    template <bool Enable, template <typename Lhs, typename Rhs> class Expr, typename Lhs, typename Rhs>
    struct new_binary_expression_impl
    {
      using type = bad_statement;
    };

    template <template <typename Lhs, typename Rhs> class Expr, typename Lhs, typename Rhs>
    struct new_binary_expression_impl<true, Expr, Lhs, Rhs>
    {
      using type = Expr<Lhs, Rhs>;
    };
  }
  template <typename Check, template <typename Lhs, typename Rhs> class Expr, typename Lhs, typename Rhs>
  using new_binary_expression_t = typename detail::new_binary_expression_impl<Check::value, Expr, Lhs, Rhs>::type;

  namespace detail
  {
    template <bool Enable, template <typename Lhs, typename... Rhs> class Expr, typename Lhs, typename... Rhs>
    struct new_nary_expression_impl
    {
      using type = bad_statement;
    };

    template <template <typename Lhs, typename... Rhs> class Expr, typename Lhs, typename... Rhs>
    struct new_nary_expression_impl<true, Expr, Lhs, Rhs...>
    {
      using type = Expr<Lhs, Rhs...>;
    };
  }
  template <typename Check, template <typename Lhs, typename... Rhs> class Expr, typename Lhs, typename... Rhs>
  using new_nary_expression_t = typename detail::new_nary_expression_impl<Check::value, Expr, Lhs, Rhs...>::type;

  // basic operators
  template <typename Expr, typename ValueType>
  struct basic_expression_operators
  {
    template <template <typename Lhs, typename Rhs> class NewExpr, typename T>
    struct _new_binary_expression
    {
      using type =
          new_binary_expression_t<check_comparison_t<Expr, wrap_operand_t<T>>, NewExpr, Expr, wrap_operand_t<T>>;
    };
    template <template <typename Lhs, typename Rhs> class NewExpr, typename T>
    using _new_binary_expression_t = typename _new_binary_expression<NewExpr, T>::type;

    // workaround for msvs bug
    //	template <template <typename Lhs, typename... Rhs> class NewExpr, typename... T>
    //    using _new_nary_expression_t =
    //        new_nary_expression_t<logic::all_t<check_comparison_t<
    //                              wrap_operand_t<T>>::value...>,
    //                              NewExpr,
    //                              Expr,
    //                              wrap_operand_t<T>...>;
    template <template <typename Lhs, typename... Rhs> class NewExpr, typename... T>
    struct _new_nary_expression
    {
      using type = new_nary_expression_t<logic::all_t<check_comparison_t<Expr, wrap_operand_t<T>>::value...>,
                                         NewExpr,
                                         Expr,
                                         wrap_operand_t<T>...>;
    };

    template <typename T>
    _new_binary_expression_t<equal_to_t, T> operator==(T t) const
    {
      using rhs = wrap_operand_t<T>;
      check_comparison_t<Expr, rhs>::_();

      return {*static_cast<const Expr*>(this), rhs{t}};
    }

    template <typename T>
    _new_binary_expression_t<not_equal_to_t, T> operator!=(T t) const
    {
      using rhs = wrap_operand_t<T>;
      check_comparison_t<Expr, rhs>::_();

      return {*static_cast<const Expr*>(this), rhs{t}};
    }

    template <typename T>
    _new_binary_expression_t<less_than_t, T> operator<(T t) const
    {
      using rhs = wrap_operand_t<T>;
      check_comparison_t<Expr, rhs>::_();

      return {*static_cast<const Expr*>(this), rhs{t}};
    }

    template <typename T>
    _new_binary_expression_t<less_equal_t, T> operator<=(T t) const
    {
      using rhs = wrap_operand_t<T>;
      check_comparison_t<Expr, rhs>::_();

      return {*static_cast<const Expr*>(this), rhs{t}};
    }

    template <typename T>
    _new_binary_expression_t<greater_than_t, T> operator>(T t) const
    {
      using rhs = wrap_operand_t<T>;
      check_comparison_t<Expr, rhs>::_();

      return {*static_cast<const Expr*>(this), rhs{t}};
    }

    template <typename T>
    _new_binary_expression_t<greater_equal_t, T> operator>=(T t) const
    {
      using rhs = wrap_operand_t<T>;
      check_comparison_t<Expr, rhs>::_();

      return {*static_cast<const Expr*>(this), rhs{t}};
    }

    is_null_t<Expr> is_null() const
    {
      return {*static_cast<const Expr*>(this)};
    }

    is_not_null_t<Expr> is_not_null() const
    {
      return {*static_cast<const Expr*>(this)};
    }

    sort_order_t<Expr, sort_type::asc> asc() const
    {
      return {*static_cast<const Expr*>(this)};
    }

    sort_order_t<Expr, sort_type::desc> desc() const
    {
      return {*static_cast<const Expr*>(this)};
    }

    // Hint: use value_list wrapper for containers...
    // workaround for msvs bug
    //	template <typename... T>
    //	_new_nary_expression_t<in_t, T...> in(T... t) const
    //	{
    //		check_rhs_in_arguments_t<Expr, wrap_operand_t<T>...>::_();
    //		return {*static_cast<const Expr*>(this), wrap_operand_t<T>{t}...};
    //	}
    template <typename... T>
    typename _new_nary_expression<in_t, T...>::type in(T... t) const
    {
      check_in_t<Expr, T...>::_();
      return {*static_cast<const Expr*>(this), typename wrap_operand<T>::type{t}...};
    }

    // workaround for msvs bug
    //	template <typename... T>
    //	_new_nary_expression_t<not_in_t, T...> not_in(T... t) const
    //	{
    //		check_rhs_in_arguments_t<Expr, wrap_operand_t<T>...>::_();
    //		return {*static_cast<const Expr*>(this), wrap_operand_t<T>{t}...};
    //	}
    template <typename... T>
    typename _new_nary_expression<not_in_t, T...>::type not_in(T... t) const
    {
      check_in_t<Expr, T...>::_();
      return {*static_cast<const Expr*>(this), typename wrap_operand<T>::type{t}...};
    }

    template <typename Defer = void>
    auto operator not() const -> return_type_not_t<Expr, Defer>
    {
      return_type_not<Expr, Defer>::check::_();
      return {*static_cast<const Expr*>(this)};
    }

    template <typename R>
    auto operator and(const R& r) const -> return_type_and_t<Expr, R>
    {
      return_type_and<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator&(const R& r) const -> return_type_bitwise_and_t<Expr, R>
    {
      return_type_bitwise_and<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator|(const R& r) const -> return_type_bitwise_or_t<Expr, R>
    {
      return_type_bitwise_or<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator or(const R& r) const -> return_type_or_t<Expr, R>
    {
      return_type_or<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator+(const R& r) const -> return_type_plus_t<Expr, R>
    {
      return_type_plus<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator-(const R& r) const -> return_type_minus_t<Expr, R>
    {
      return_type_minus<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator*(const R& r) const -> return_type_multiplies_t<Expr, R>
    {
      return_type_multiplies<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator/(const R& r) const -> return_type_divides_t<Expr, R>
    {
      return_type_divides<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename R>
    auto operator%(const R& r) const -> return_type_modulus_t<Expr, R>
    {
      return_type_modulus<Expr, R>::check::_();
      return {*static_cast<const Expr*>(this), wrap_operand_t<R>{r}};
    }

    template <typename Defer = void>
    auto operator+() const -> return_type_unary_plus_t<Expr, Defer>
    {
      return_type_unary_plus<Expr, Defer>::check::_();
      return {*static_cast<const Expr*>(this)};
    }

    template <typename Defer = void>
    auto operator-() const -> return_type_unary_minus_t<Expr, Defer>
    {
      return_type_unary_minus<Expr, Defer>::check::_();
      return {*static_cast<const Expr*>(this)};
    }
  };
}

#endif
