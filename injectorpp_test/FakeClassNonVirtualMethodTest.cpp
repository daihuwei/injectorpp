#include "gmock/gmock.h"
#include "Injector.h"
#include "Fixtures/Address.h"

class FakeClassNonVirtualMethodTestFixture : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        INJECTORPP_SETUP();
    }

    virtual void TearDown()
    {
        INJECTORPP_CLEANUP();
    }
};

TEST_F(FakeClassNonVirtualMethodTestFixture, FakeFunctionWithIntReturnDefaultValue)
{
    // Prepare
    Address* fakeAddress = INJECTORPP_FAKE<Address>();
    int expected = 0;

    // Act
    int actual = fakeAddress->GetZipCode();

    // Assert
    EXPECT_EQ(expected, actual);
}

TEST_F(FakeClassNonVirtualMethodTestFixture, FakeFunctionWithCCharReturnDefaultValue)
{
    // Prepare
    Address* fakeAddress = INJECTORPP_FAKE<Address>();
    char* expected = "";

    // Act
    char* actual = fakeAddress->GetCity();

    // Assert
    EXPECT_EQ(expected, actual);
}

TEST_F(FakeClassNonVirtualMethodTestFixture, FakeFunctionWithStringReturnDefaultValue)
{
    // Prepare
    Address* fakeAddress = INJECTORPP_FAKE<Address>();
    std::string expected = "";

    // Act
    std::string actual = fakeAddress->GetCapitalCityByCountry("");

    // Assert
    EXPECT_EQ(expected, actual);
}