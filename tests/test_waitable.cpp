#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <ratio>
#include <thread>
#include <vector>
#include <future>
#include "../include/waitable.h"


TEST_CASE("Default constructable"){
    Waitable<int>();
    Waitable<std::string>();
    Waitable<std::vector<int>>();
}

TEST_CASE("Initial value is used"){
    REQUIRE(Waitable<int>(123).get_no_block() == 123);
    REQUIRE(Waitable<std::string>("lol").get_no_block() == "lol");
    REQUIRE(Waitable<int>().get_no_block() == 0);
    REQUIRE(Waitable<std::string>().get_no_block() == "");
}

TEST_CASE("Get blocks"){
    Waitable<int> w;
    SECTION("Single consumer"){
        auto blocked = std::async(std::launch::async, [&]{w.get();});
        auto status = blocked.wait_for(ms(1));
        REQUIRE(status == std::future_status::timeout);
        w.release();
    }
    //SECTION("Multiple consumers"){
    //    auto blocked = {

    //    }
    //}
}

TEST_CASE("Release releases"){
}
