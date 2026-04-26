#include <gtest/gtest.h>
#include "Account.h"

class AccountTest : public ::testing::Test {
protected:
    void SetUp() override {
        account = new Account(1, 100);
    }
    
    void TearDown() override {
        delete account;
    }
    
    Account* account;
};

TEST_F(AccountTest, ConstructorInitializesCorrectly) {
    EXPECT_EQ(1, account->id());
    EXPECT_EQ(100, account->GetBalance());
}

TEST_F(AccountTest, GetBalanceReturnsCorrectValue) {
    EXPECT_EQ(100, account->GetBalance());
}

TEST_F(AccountTest, LockThrowsWhenAlreadyLocked) {
    account->Lock();
    EXPECT_THROW(account->Lock(), std::runtime_error);
}

TEST_F(AccountTest, LockAndUnlockWorkCorrectly) {
    EXPECT_NO_THROW(account->Lock());
    EXPECT_NO_THROW(account->Unlock());
}

TEST_F(AccountTest, ChangeBalanceThrowsWhenNotLocked) {
    EXPECT_THROW(account->ChangeBalance(50), std::runtime_error);
}

TEST_F(AccountTest, ChangeBalanceWorksWhenLocked) {
    account->Lock();
    EXPECT_NO_THROW(account->ChangeBalance(50));
    EXPECT_EQ(150, account->GetBalance());
}

TEST_F(AccountTest, ChangeBalanceWithNegativeDiff) {
    account->Lock();
    account->ChangeBalance(-30);
    EXPECT_EQ(70, account->GetBalance());
}

TEST_F(AccountTest, MultipleOperations) {
    account->Lock();
    account->ChangeBalance(50);
    account->ChangeBalance(-20);
    account->Unlock();
    EXPECT_EQ(130, account->GetBalance());
}
