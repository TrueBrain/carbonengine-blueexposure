class BluePyStrTest : public testing::TestWithParam<std::string_view>
{
protected:
	void SetUp() override;

	void TearDown() override;
};