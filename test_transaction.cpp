#include <gtest/gtest.h>
#include <trompeloeil.hpp>
#include "Transaction.h"
#include "Account.h"

class MockAccount : public Account {
public:
    using Account::Account;
    
    MAKE_MOCK0(GetBalance, int(), const override);
    MAKE_MOCK1(ChangeBalance, void(int), override);
    MAKE_MOCK0(Lock, void(), override);
    MAKE_MOCK0(Unlock, void(), override);
    MAKE_MOCK0(GetId, int());
};

class TestableTransaction : public Transaction {
public:
    using Transaction::Transaction;
    
    MAKE_MOCK3(SaveToDataBase, void(Account&, Account&, int), override);
};

template<typename Mock>
void SetupId(Mock& mock, int id) {
    ALLOW_CALL(mock, GetId()).RETURN(id);
}

TEST(TransactionTest, MakeThrowsWhenSameAccount) {
    MockAccount acc(1, 100);
    SetupId(acc, 1);
    EXPECT_THROW(Transaction().Make(acc, acc, 200), std::logic_error);
}

TEST(TransactionTest, MakeThrowsWhenNegativeSum) {
    MockAccount from(1, 100);
    MockAccount to(2, 50);
    SetupId(from, 1);
    SetupId(to, 2);
    EXPECT_THROW(Transaction().Make(from, to, -50), std::invalid_argument);
}

TEST(TransactionTest, MakeThrowsWhenSumLessThan100) {
    MockAccount from(1, 100);
    MockAccount to(2, 50);
    SetupId(from, 1);
    SetupId(to, 2);
    EXPECT_THROW(Transaction().Make(from, to, 99), std::logic_error);
}

TEST(TransactionTest, MakeReturnsFalseWhenFeeTooHigh) {
    MockAccount from(1, 100);
    MockAccount to(2, 50);
    SetupId(from, 1);
    SetupId(to, 2);
    Transaction t;
    t.set_fee(60);
    EXPECT_FALSE(t.Make(from, to, 100));
}

TEST(TransactionTest, MakeSuccessWithEnoughBalance) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);
    
    SetupId(from, 1);
    SetupId(to, 2);
    
    REQUIRE_CALL(from, Lock());
    REQUIRE_CALL(to, Lock());
    
    REQUIRE_CALL(to, ChangeBalance(200));
    REQUIRE_CALL(to, GetBalance()).RETURN(500);
    REQUIRE_CALL(to, ChangeBalance(-201));
    
    REQUIRE_CALL(to, Unlock());
    REQUIRE_CALL(from, Unlock());
    
    Transaction t;
    t.set_fee(1);
    EXPECT_TRUE(t.Make(from, to, 200));
}

TEST(TransactionTest, MakeFailsWhenNotEnoughBalance) {
    MockAccount from(1, 100);
    MockAccount to(2, 50);
    
    SetupId(from, 1);
    SetupId(to, 2);
    
    REQUIRE_CALL(from, Lock());
    REQUIRE_CALL(to, Lock());
    
    REQUIRE_CALL(to, ChangeBalance(200));
    REQUIRE_CALL(to, GetBalance()).RETURN(50);
    
    REQUIRE_CALL(to, Unlock());
    REQUIRE_CALL(from, Unlock());
    
    Transaction t;
    t.set_fee(1);
    EXPECT_FALSE(t.Make(from, to, 200));
}

TEST(TransactionTest, SaveToDataBaseIsCalled) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);
    TestableTransaction t;
    
    SetupId(from, 1);
    SetupId(to, 2);
    
    REQUIRE_CALL(from, Lock());
    REQUIRE_CALL(to, Lock());
    ALLOW_CALL(to, ChangeBalance(trompeloeil::_));
    ALLOW_CALL(to, GetBalance()).RETURN(500);
    REQUIRE_CALL(to, Unlock());
    REQUIRE_CALL(from, Unlock());
    
    REQUIRE_CALL(t, SaveToDataBase(trompeloeil::_, trompeloeil::_, 200)).TIMES(1);
    
    t.Make(from, to, 200);
}

TEST(TransactionTest, DebitReturnsFalseWhenInsufficientBalance) {
    MockAccount acc(1, 50);
    TestableTransaction t;
    
    ALLOW_CALL(acc, GetBalance()).RETURN(50);
    
    struct T : Transaction { using Transaction::Debit; };
    T t2;
    EXPECT_FALSE(t2.Debit(acc, 100));
}

TEST(TransactionTest, DebitReturnsTrueWhenSufficientBalance) {
    MockAccount acc(1, 200);
    
    ALLOW_CALL(acc, GetBalance()).RETURN(200);
    REQUIRE_CALL(acc, ChangeBalance(-100));
    
    struct T : Transaction { using Transaction::Debit; };
    T t;
    EXPECT_TRUE(t.Debit(acc, 100));
}

TEST(TransactionTest, CreditChangesBalance) {
    MockAccount acc(1, 100);
    
    REQUIRE_CALL(acc, ChangeBalance(50));
    
    struct T : Transaction { using Transaction::Credit; };
    T t;
    t.Credit(acc, 50);
}

TEST(TransactionTest, FeeGetterAndSetterWork) {
    Transaction t;
    EXPECT_EQ(1, t.fee());
    t.set_fee(10);
    EXPECT_EQ(10, t.fee());
}
