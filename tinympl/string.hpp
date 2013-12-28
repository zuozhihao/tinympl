/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Ennio Barbaro <e.barbaro@sssup.it>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TINYMPL_STRING_HPP
#define TINYMPL_STRING_HPP

#include "algorithm.hpp"
#include "sequence.hpp"

namespace tinympl
{

template<class T,T ... chars>
struct basic_string
{	
	enum {size = sizeof ...( chars )};
	enum {emtpy = ( size == 0 )};
	
	typedef basic_string<T,chars...> type;
	typedef T value_type;
	typedef const T * const_pointer;
	
	constexpr static value_type at(std::size_t i) {return c_str()[i];}
	constexpr static const_pointer c_str() {return v_;}
	constexpr static value_type front() {return at(0);}
	constexpr static value_type back() {return at(size - 1);}
	
private:
	template<T t>
	using wrap = std::integral_constant<T,t>;
	
	template<class ... Us>
	using unwrap = basic_string<T, Us::value ... >;
	
	template<class ... Strings> struct merge;
	template<T ... pre,T ... post> struct merge<basic_string<T,pre...>, basic_string<T,post...> >
	{
		typedef basic_string<T,pre..., post...> type;
	};
	
	template<class A,class B,class ... Others> struct merge<A,B,Others...>
	{
		typedef typename merge< typename merge<A,B>::type, Others ... >::type type;
	};
	
	template<class A> struct merge<A>
	{
		typedef A type;
	};
	
public:

	template<std::size_t pos,class Str>
	using insert = tinympl::insert<pos, 
			Str,
			basic_string<T,chars...> >;
	
	template<std::size_t pos,T ... NewChars>
	using insert_c = insert<pos, basic_string<T,NewChars...> >;
	
	template<std::size_t pos,std::size_t count>
	using erase = tinympl::erase<pos,pos + count,
		basic_string<T,chars...> >;
	
	template<T ... NewChars>
	using append_c = insert_c<size,NewChars...>;
	
	template<class Str>
	using append = insert<size,Str>;
	
	template<std::size_t pos, std::size_t count>
	class substr
	{
		static_assert( pos <= size, "substr pos out of range ");
		static_assert( pos + count <= size, "substr count out of range");
		
		template<class ... Args>
		using erase_head_t = typename variadic::erase<0,pos, unwrap, Args... >::type;
	
	public:
		typedef typename variadic::erase<pos+count,size, erase_head_t, wrap<chars> ... >::type type;
	};

	template<class OtherStr>
	using compare = lexicographical_compare<
		basic_string<T,chars...>,
		OtherStr>;
	
private:
	static constexpr T v_[size + 1] = {chars ... , 0};
};

template<class T, const T * ptr>
struct make_basic_string
{
	template<T ... ts> struct extract
	{
		typedef typename std::conditional<
			ptr[ sizeof ... (ts) ] == 0,
			basic_string<T, ts...>,
			extract<ts..., ptr[sizeof ... (ts) ]>
		>::type::type type;
	};
	using type = typename extract<>::type;
};

template<class T>
using make_mpl_string = typename make_basic_string<typename T::type,T::value>::type;

template<const char * p>
using string = typename make_basic_string<char,p>::type;

#define TINYMPL_STRING_JOIN2(arg1,arg2) TINYMPL_DO_STRING_JOIN2(arg1,arg2)
#define TINYMPL_DO_STRING_JOIN2(arg1,arg2) arg1 ## arg2

#define MAKE_TINYMPL_STRING(name,str) \
	struct TINYMPL_STRING_JOIN2(tinympl_string_temporary_, name) { \
		typedef char type; \
		static constexpr const char value[] = str; \
	}; \
	typedef make_mpl_string<TINYMPL_STRING_JOIN2(tinympl_string_temporary_, name)> name

template<class T,T ... ts> struct as_sequence<basic_string<T,ts...> >
{
	template<class ... Args>
	struct do_rebind
	{
		template<class U>
		using check_t = std::is_same<typename U::value_type,T>;
		
		static_assert(variadic::all_of<check_t,Args...>::value, 
					  "basic_string: unable to rebind when the arguments are not of the same value_type" );
		
		typedef basic_string<T, Args::value ...> type;
	};

	typedef sequence< std::integral_constant<T,ts> ...> type; 
	template<class ... Ts> using rebind = typename do_rebind<Ts...>::type;
};

}

#endif // TINYMPL_STRING_HPP 
