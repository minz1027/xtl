/** @file
memory mapped files system and unit tests
@copyright David Mott (c) 2016. Distributed under the Boost Software License Version 1.0. See LICENSE.md or http://boost.org/LICENSE_1_0.txt for details.
*/

#include <xtd/mapped_file.hpp>
#include <xtd/executable.hpp>

class test_mapped_file : public ::testing::Test{
public:
  static void SetUpTestCase(){}

  static void TearDownTestCase() {
    xtd::filesystem::remove(xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename());
  }
};

struct mapped_file_test_struct{
  int age;
  char first_name[50];
  char last_name[50];
  int ssn;
};

TEST_F(test_mapped_file, initialization){
  auto oPath = xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename();
  EXPECT_NO_THROW(xtd::mapped_file oFile(oPath));
  xtd::filesystem::remove(oPath);
}

TEST_F(test_mapped_file, page_initialization){
  auto oPath = xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename();
  xtd::mapped_file oFile(xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename());
  EXPECT_NO_THROW(auto oPage = oFile.get<mapped_file_test_struct>(0));
  EXPECT_NO_THROW(auto oPage2 = oFile.get<mapped_file_test_struct>(1));
  EXPECT_NO_THROW(auto oPage3 = oFile.get<mapped_file_test_struct>(2));
  xtd::filesystem::remove(oPath);
}

TEST_F(test_mapped_file, read){
  auto oPath = xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename();
  xtd::mapped_file oFile(oPath);
  auto oPage = oFile.get<mapped_file_test_struct>(0);
  mapped_file_test_struct s;
  memcpy(&s, oPage.get(), sizeof(mapped_file_test_struct));
  xtd::filesystem::remove(oPath);
}

TEST_F(test_mapped_file, write){
  auto oPath = xtd::filesystem::temp_directory_path() + xtd::executable::this_executable().path().filename();
  xtd::mapped_file oFile(oPath);
  {
    auto oPage = oFile.get<mapped_file_test_struct>(0);
    oPage->age=123;
    strcpy(oPage->first_name, "Groucho");
    strcpy(oPage->last_name, "Marx");
    oPage->ssn = 456;
  }
  {
    auto oPage = oFile.get<mapped_file_test_struct>(0);
    EXPECT_EQ(oPage->age, 123);
    EXPECT_STREQ(oPage->first_name, "Groucho");
    EXPECT_STREQ(oPage->last_name, "Marx");
    EXPECT_EQ(oPage->ssn, 456);
  }
  xtd::filesystem::remove(oPath);

}
