/**
 * \copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
 */

#include <xtd.hpp>

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
#pragma warning(push, 0)
#endif

#include "gtest/gtest.h"

#if (XTD_COMPILER_MSVC & XTD_COMPILER)
#pragma warning(pop)
#endif


#include "test_callback.hpp"
#include "test_data_convert.hpp"
#include "test_dynamic_library.hpp"
#include "test_event_trace.hpp"
#include "test_exception.hpp"
#include "test_executable.hpp"
#include "test_logging.hpp"
#include "test_meta.hpp"
#include "test_parse.hpp"
#include "test_path.hpp"
#include "test_socket.hpp"
#include "test_string.hpp"
#if (XTD_HAS_UUID)
  #include "test_unique_id.hpp"
#endif
#include "test_var.hpp"


int main(int argc, char *argv[]){
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
