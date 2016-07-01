/** @file
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd.hpp>

namespace{
  thread_local std::stack<std::string> _ThreadStack;
  thread_local bool _InTrace = false;
}

extern "C"{
  void __xtd_EventEnter(void *){
    if (_InTrace) return;
    _InTrace = true;
    _ThreadStack.push("");
    _InTrace = false;
  }

  void __xtd_EventLeave(void *){
    if (_InTrace) return;
    _InTrace = true;
    _ThreadStack.pop();
    _InTrace = false;
  }
}
