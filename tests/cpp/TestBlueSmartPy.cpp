#include <BlueSmartPy.h>

class BluePyStrTest : public testing::TestWithParam<std::string_view> {};

TEST_P(BluePyStrTest, StrConversion)
{
    auto param = GetParam();
    EXPECT_STREQ( param.data(), BluePyStr(param.data()).Str() );
}

TEST_P(BluePyStrTest, CStrConversion)
{
    auto param = GetParam();
    EXPECT_STREQ( param.data(), BluePyStr(param.data()).CStr() );
}

#include "blns.cpp"

INSTANTIATE_TEST_SUITE_P(StrFromStr, BluePyStrTest, testing::ValuesIn(s_BigListOfNaughtyStrings));
