/** @file
  handle necessary filesystem and path functionality until C++17 is finalized
  \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.

*/


#pragma once

#include <xtd.hpp>
#include <xtd/string.hpp>

namespace xtd{

  namespace socket{
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    using path_value_type = tchar;
#else
    using path_value_type = char;
#endif
  }
  
  struct path : xtd::xstring<socket::path_value_type >{
    using string_type = xtd::xstring<socket::path_value_type >;
#if ((XTD_OS_WINDOWS | XTD_OS_MINGW) & XTD_OS)
    static const value_type seperator = __('\\');
#else
    static const value_type seperator = '/';
#endif

    inline operator string_type() const{ return *this; }
    inline const string_type& native() const{ return *this; }

    string_type& string() { return *this; }
    const string_type& string() const { return *this; }

    path() : string_type(){}
    explicit path(const path& src) : string_type(src){}
    explicit path(path&& src) : string_type(std::move(src)) {}

    template <typename _Ty> path(const _Ty& src) : string_type(src){}

    path& operator=(path&& src) {
      static_cast<string_type&>(*this) = std::move(src);
      return *this;
    }

    template <typename _Ty> path& append(const _Ty& src){
      if (seperator != back() && seperator != src[0]){
        *this += seperator;
      }else if (seperator == back() && seperator == src[0]){
        pop_back();
      }
      *this += src;
      return *this;
    }

    template <typename _Ty> path& operator/=(const _Ty& src) { return append(src); }

    path& replace_filename(const path& src){
      TODO("path::replace_filename needs a lot of work to be TS compliant");
      resize(1+find_last_of(seperator));
      return append(src);
    }

    path& make_preferred(){
#if ((XTD_OS_CYGWIN|XTD_OS_UNIX) & XTD_OS)
        char non_preferred_seperator = '\\';
#elif ((XTD_OS_WINDOWS|XTD_OS_MINGW) & XTD_OS)
      char non_preferred_seperator = '/';
#else
      static_assert(false, "Platform not supported");
#endif
      for (size_type i=0 ; i<size() ; i++){
        if ((*this)[i] == non_preferred_seperator){
          (*this)[i] = seperator;
        }
      }
      return *this;
    }
  };

}