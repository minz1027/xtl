/** @file
 * @copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd/xtd.hpp>


namespace{
  xtd::concurrent::hash_map<void*,std::string> _FunctionNames;
  thread_local std::stack<std::string> _ThreadStack;
  thread_local bool _InTrace = false;
}

extern "C"{
  void __xtd_EventEnter(void *){
    if (_InTrace){
      return;
    }
/*    Dl_info oInfo;
    if (!_FunctionNames.exists(pAddr) && dladdr(pAddr, &oInfo)){
      _FunctionNames.insert(pAddr, oInfo.dli_sname);
    }*/
    _InTrace = true;
  //  _ThreadStack.push("");
    _InTrace = false;
  }

  void __xtd_EventLeave(void *){
    if (_InTrace){
      return;
    }
    _InTrace = true;
  //  _ThreadStack.pop();
    _InTrace = false;
  }
}
