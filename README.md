# Homework

## Бейджи для статуса работы и покрытия:

[![CI](https://github.com/bashkirgreg/banking/actions/workflows/ci.yml/badge.svg)](https://github.com/bashkirgreg/banking/actions/workflows/ci.yml)
[![Coverage Status](https://coveralls.io/repos/github/bashkirgreg/banking/badge.svg?branch=main)](https://coveralls.io/github/bashkirgreg/banking?branch=main) 

## Задание
1. Создайте `CMakeList.txt` для библиотеки *banking*.
2. Создайте модульные тесты на классы `Transaction` и `Account`.
    * Используйте mock-объекты.
    * Покрытие кода должно составлять 100%.
3. Настройте сборочную процедуру на **GitHub Actions**.
4. Настройте [Coveralls.io](https://coveralls.io/).

## Шаги выполнения:
1)Создаём и переходим в новую директорию:
```sh
$ cd ~/bashkirgreg/workspace/projects
$ mkdir banking && cd banking
```

2)Наполняем её слегка иными файлами из библиотеки `banking` через стандартный `cat`:
```sh
#pragma once
class Account {
 public:
  Account(int id, int balance);
  virtual ~Account();

  virtual int GetBalance() const;
  virtual void ChangeBalance(int diff);
  virtual void Lock();
  virtual void Unlock();
  virtual int id() const { return id_; }

 private:
  int id_;
  int balance_;
  bool is_locked_;
};
```
```sh
#include "Account.h"

#include <stdexcept>

Account::Account(int id, int balance)
    : id_(id), balance_(balance), is_locked_(false) {}

Account::~Account() {}

int Account::GetBalance() const { return balance_; }

void Account::ChangeBalance(int diff) {
  if (!is_locked_) throw std::runtime_error("at first lock the account");
  balance_ += diff;
}

void Account::Lock() {
  if (is_locked_) throw std::runtime_error("already locked");
  is_locked_ = true;
}

void Account::Unlock() { is_locked_ = false; }
```
```sh
#pragma once

class Account;

class Transaction {
 public:
  Transaction();
  virtual ~Transaction();

  bool Make(Account& from, Account& to, int sum);
  int fee() const { return fee_; }
  void set_fee(int fee) { fee_ = fee; }

 protected:
  void Credit(Account& accout, int sum);
  bool Debit(Account& accout, int sum);

 private:
  virtual void SaveToDataBase(Account& from, Account& to, int sum);

  int fee_;
};
```
```sh
#include "Transaction.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Account.h"

namespace {
struct Guard {
  Guard(Account& account) : account_(&account) { account_->Lock(); }
  ~Guard() { account_->Unlock(); }
 private:
  Account* account_;
};
}

Transaction::Transaction() : fee_(1) {}
Transaction::~Transaction() {}

bool Transaction::Make(Account& from, Account& to, int sum) {
  if (from.id() == to.id()) throw std::logic_error("invalid action");
  if (sum < 0) throw std::invalid_argument("sum can't be negative");
  if (sum < 100) throw std::logic_error("too small");
  if (fee_ * 2 > sum) return false;

  Guard guard_from(from);
  Guard guard_to(to);

  Credit(to, sum);
  bool success = Debit(to, sum + fee_);
  if (!success) to.ChangeBalance(-sum);

  SaveToDataBase(from, to, sum);
  return success;
}

void Transaction::Credit(Account& accout, int sum) {
  assert(sum > 0);
  accout.ChangeBalance(sum);
}

bool Transaction::Debit(Account& accout, int sum) {
  assert(sum > 0);
  if (accout.GetBalance() > sum) {
    accout.ChangeBalance(-sum);
    return true;
  }
  return false;
}

void Transaction::SaveToDataBase(Account& from, Account& to, int sum) {
  std::cout << from.id() << " send to " << to.id() << " $" << sum << std::endl;
  std::cout << "Balance " << from.id() << " is " << from.GetBalance()
            << std::endl;
  std::cout << "Balance " << to.id() << " is " << to.GetBalance() << std::endl;
}
```

3)Создаём `CMakeLists.txt` для всего этого:
```sh
cmake_minimum_required(VERSION 3.14)
project(banking)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)
option(COVERAGE "Enable coverage reporting" OFF)

if(COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0 -g")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -O0 -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

add_library(banking STATIC
    Account.cpp
    Transaction.cpp
)

target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTS)
    enable_testing()
    
    include(FetchContent)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    
    FetchContent_Declare(
        trompeloeil
        GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
        GIT_TAG v43
    )
    
    FetchContent_MakeAvailable(googletest trompeloeil)
    
    add_executable(banking_test
        test_account.cpp
        test_transaction.cpp
    )
    
    target_link_libraries(banking_test PRIVATE
        banking
        gtest_main
        trompeloeil
    )
    
    target_include_directories(banking_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${trompeloeil_SOURCE_DIR}/include
    )
    
    add_test(NAME banking_test COMMAND banking_test)
endif()
```

4)Аналогичным образом создаём `test_account.cpp` и `test_transaction.cpp` для будущих тестов:
```sh
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
```
```sh
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
    MAKE_MOCK0(id, int(), const override);
};

class TestableTransaction : public Transaction {
public:
    using Transaction::Transaction;
    
    MAKE_MOCK3(SaveToDataBase, void(Account&, Account&, int), override);
    
    void DoCredit(Account& acc, int sum) { Credit(acc, sum); }
    bool DoDebit(Account& acc, int sum) { return Debit(acc, sum); }
};

TEST(TransactionTest, MakeThrowsWhenSameAccount) {
    MockAccount acc(1, 100);
    ALLOW_CALL(acc, id()).RETURN(1);
    EXPECT_THROW(Transaction().Make(acc, acc, 200), std::logic_error);
}

TEST(TransactionTest, MakeThrowsWhenNegativeSum) {
    MockAccount from(1, 100);
    MockAccount to(2, 50);
    ALLOW_CALL(from, id()).RETURN(1);
    ALLOW_CALL(to, id()).RETURN(2);
    EXPECT_THROW(Transaction().Make(from, to, -50), std::invalid_argument);
}

TEST(TransactionTest, MakeThrowsWhenSumLessThan100) {
    MockAccount from(1, 100);
    MockAccount to(2, 50);
    ALLOW_CALL(from, id()).RETURN(1);
    ALLOW_CALL(to, id()).RETURN(2);
    EXPECT_THROW(Transaction().Make(from, to, 99), std::logic_error);
}

TEST(TransactionTest, MakeReturnsFalseWhenFeeTooHigh) {
    MockAccount from(1, 100);
    MockAccount to(2, 50);
    ALLOW_CALL(from, id()).RETURN(1);
    ALLOW_CALL(to, id()).RETURN(2);
    Transaction t;
    t.set_fee(60);
    EXPECT_FALSE(t.Make(from, to, 100));
}

TEST(TransactionTest, MakeSuccessWithEnoughBalance) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);
    TestableTransaction t;
    
    ALLOW_CALL(from, id()).RETURN(1);
    ALLOW_CALL(to, id()).RETURN(2);
    
    REQUIRE_CALL(from, Lock());
    REQUIRE_CALL(to, Lock());
    
    REQUIRE_CALL(to, ChangeBalance(200));
    REQUIRE_CALL(to, GetBalance()).RETURN(500);
    REQUIRE_CALL(to, ChangeBalance(-201));
    
    REQUIRE_CALL(to, Unlock());
    REQUIRE_CALL(from, Unlock());
    
    ALLOW_CALL(t, SaveToDataBase(trompeloeil::_, trompeloeil::_, trompeloeil::_));
    
    t.set_fee(1);
    EXPECT_TRUE(t.Make(from, to, 200));
}

TEST(TransactionTest, MakeFailsWhenNotEnoughBalance) {
    MockAccount from(1, 100);
    MockAccount to(2, 500);
    TestableTransaction t;
    
    ALLOW_CALL(from, id()).RETURN(1);
    ALLOW_CALL(to, id()).RETURN(2);
    
    REQUIRE_CALL(from, Lock());
    REQUIRE_CALL(to, Lock());
    
    REQUIRE_CALL(to, ChangeBalance(200));
    REQUIRE_CALL(to, GetBalance()).RETURN(50);
    REQUIRE_CALL(to, ChangeBalance(-200));
    
    REQUIRE_CALL(to, Unlock());
    REQUIRE_CALL(from, Unlock());
    
    ALLOW_CALL(t, SaveToDataBase(trompeloeil::_, trompeloeil::_, trompeloeil::_));
    
    t.set_fee(1);
    EXPECT_FALSE(t.Make(from, to, 200));
}

TEST(TransactionTest, SaveToDataBaseIsCalled) {
    MockAccount from(1, 1000);
    MockAccount to(2, 500);
    TestableTransaction t;
    
    ALLOW_CALL(from, id()).RETURN(1);
    ALLOW_CALL(to, id()).RETURN(2);
    
    REQUIRE_CALL(from, Lock());
    REQUIRE_CALL(to, Lock());
    REQUIRE_CALL(to, ChangeBalance(100));
    ALLOW_CALL(to, GetBalance()).RETURN(500);
    REQUIRE_CALL(to, ChangeBalance(-101));
    REQUIRE_CALL(to, Unlock());
    REQUIRE_CALL(from, Unlock());
    
    REQUIRE_CALL(t, SaveToDataBase(trompeloeil::_, trompeloeil::_, 100)).TIMES(1);
    
    t.set_fee(1);
    t.Make(from, to, 100);
}

TEST(TransactionTest, DebitReturnsFalseWhenInsufficientBalance) {
    MockAccount acc(1, 50);
    TestableTransaction t;
    
    ALLOW_CALL(acc, GetBalance()).RETURN(50);
    
    EXPECT_FALSE(t.DoDebit(acc, 100));
}

TEST(TransactionTest, DebitReturnsTrueWhenSufficientBalance) {
    MockAccount acc(1, 200);
    TestableTransaction t;
    
    ALLOW_CALL(acc, GetBalance()).RETURN(200);
    REQUIRE_CALL(acc, ChangeBalance(-100));
    
    EXPECT_TRUE(t.DoDebit(acc, 100));
}

TEST(TransactionTest, CreditChangesBalance) {
    MockAccount acc(1, 100);
    TestableTransaction t;
    
    REQUIRE_CALL(acc, ChangeBalance(50));
    
    t.DoCredit(acc, 50);
}

TEST(TransactionTest, FeeGetterAndSetterWork) {
    Transaction t;
    EXPECT_EQ(1, t.fee());
    t.set_fee(10);
    EXPECT_EQ(10, t.fee());
}
```

5)Теперь создаём `.github/workflows/ci.yml` и `.github/workflows/coverage.yml` для тестов и покрытия:
```sh
name: CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Configure
      run: cmake -B build -DBUILD_TESTS=ON
    
    - name: Build
      run: cmake --build build
    
    - name: Test
      working-directory: build
      run: ctest --output-on-failure
```
```sh
name: Coverage

on: [push, pull_request]

jobs:
  coverage:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y lcov
        pip install --user cpp-coveralls
    
    - name: Configure with coverage
      run: |
        rm -rf build
        cmake -B build -DBUILD_TESTS=ON -DCMAKE_CXX_FLAGS="--coverage"
    
    - name: Build
      run: cmake --build build
    
    - name: Run tests
      working-directory: build
      run: ctest
    
    - name: Collect coverage
      run: |
        cd build
        lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch
        lcov --remove coverage.info '/usr/*' '*/_deps/*' '*/test_*' --output-file coverage.info --ignore-errors unused
        find . -name "*.gcda" -exec rm -f {} \;
        find . -name "*.gcno" -exec rm -f {} \;
        rm -rf CMakeFiles
        lcov --list coverage.info
    
    - name: Upload to Coveralls
      run: |
        coveralls --gcov-options '\-lp' --build-root build --exclude _deps --exclude test --exclude /usr/include
      env:
        COVERALLS_REPO_TOKEN: ${{ secrets.COVERALLS_REPO_TOKEN }}
```

6)Публикуем всё на удалённый репозиторий из терминала через стандартные команды `Git`


7)Проверяем все на работоспособность и корректность (Очень много раз...), ожидая успешной работы `ci.yml` и работы `coverage.yml` с ошибкой в конце, поскольку у нас нет пока токена для проверки покрытия


8)Проходим регистрацию на `Coveralls.io`, получая особый токен `COVERALLS_REPO_TOKEN` для проверки покрытия, который вскоре помещаем в `Secrets and variables -> Actions -> New repository secret` для устранения ошибки в работе с `coverage.yml`


9)Ожидаем завершение двух работ на `GitHub Actions`, в случае чего исправляя ошибки и некорректировки в коде (Их было очень много...)


10)Добавляем `README.md` с нужными бейджами с сайта `Coveralls.io` для красивого отслеживания прогресса

>Сначала я пытался использовать стандартный `#include <gmock/gmock.h>`. Очень долго мучился с его настройкой, поскольку то версии конфликтовали, то не собиралось ничего, то ошибки на 50+ строк вылетали. Отчаявшись, я решил поблуждать по самым разным статьям в интернете на тему моковых объектов, и среди таких уникальных вещей, как `CxxMock` и `Rhino.Mocks`, я нашёл удивительную библиотеку `trompeloeil`, которая позволяет создавать моковые объекты и удобно с ними работать. Благодаря этому, я сумел без возникновения ошибок разместить все моковые объекты прямо в тестовом файле `test_transaction.cpp`. В конечном счёте, спустя бесчисленное множество попыток, всё вроде заработало. Как говорилось, это поистине самая сложная лабораторная работа...
