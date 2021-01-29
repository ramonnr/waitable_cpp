#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../include/blocking_queue.h"
#include <thread>
#include <array>
#include <future>

using namespace blockingqueue;

TEST_CASE( "Max size is adhered to"){
    BlockingQueue<int> bq(1);
    REQUIRE(bq.size() == 0);
    bq.push(1);
    REQUIRE(bq.size() == 1);
    bq.push(2);
    REQUIRE(bq.size() == 1);

    BlockingQueue<int> bq_bigger(2);
    for(int i = 0; i < 10; i++){
        bq_bigger.push(i);
        REQUIRE(bq_bigger.size() <= 2);
    }
}

TEST_CASE( "Overwrites from front"){
    BlockingQueue<int> bq(2);
    for(int i = 0; i < 10; i++){
        bq.push(i);
    }
    REQUIRE( bq.pop() == 8 );
    REQUIRE( bq.pop() == 9 );
}

TEST_CASE( "Single producer/consumer" ){
    BlockingQueue<int> bq;
    auto t = std::thread([&]{
        for(int i = 0; i < 20; i++){
            bq.push(i);
        }
    });

    auto all_meets_expected = true;
    for(int i = 0; i < 20; i++){
        all_meets_expected &= bq.pop() == i;
    }
    REQUIRE(all_meets_expected);
    t.join();
}

TEST_CASE( "Single producer multiple consumers" ){
   BlockingQueue<int> bq;

    std::array<std::thread, 4> consumers;
    for(int i = 0; i < 4; i++){
        consumers[i] = std::thread([&]{
                    for(int i = 0; i < 25; i++){
                        bq.pop();
                    }});
    }

    for(int i = 0; i < 100; i++){
        bq.push(i);
    }
    for(int i = 0; i < consumers.size(); i++){
        consumers[i].join();
    }
}

TEST_CASE( "Multiple producers single consumer" ){
    BlockingQueue<int> bq;
    std::array<std::thread, 4> producers;
    for(int i = 0; i < producers.size(); i++){
        producers[i] = std::thread([&]{
            for(int i = 0; i < 25; i++){
                bq.push(i);
            }
        });
    }
    for(int i = 0; i < producers.size() * 25; i++){
        bq.pop();
    }
    for(int i = 0; i < producers.size(); i++){
        producers[i].join();
    }
    REQUIRE( bq.size() == 0 );
}


TEST_CASE( "Testing that internal ref in pop is all right" ){

    BlockingQueue<std::string> bq;
    auto cmp = "Hello testing world";
    bq.push(cmp);

    REQUIRE( bq.pop() == cmp );

}

TEST_CASE( "Multiple producers, multiple consumers" ){

    std::array<bool, 100> visited;
    for(auto i = 0; i < visited.size(); i++){
        visited[i] = false;
    }
    BlockingQueue<int> bq;

    auto producer = [&](auto begin, auto iterations){
        for(auto i = begin; (i - begin) < iterations; i++){
           bq.push(i); 
        }
    };
    auto consumer = [&](auto begin, auto iterations){
        for(auto i = begin; (i - begin) < iterations; i++){
            visited[*bq.pop()] = true;
        }
    };
    
    std::array<std::thread, 4> consumers;
    std::array<std::thread, 4> producers;

    for(auto i = 0; i < 4; i++){
        consumers[i] = std::thread(consumer, 25*i, 25);
        producers[i] = std::thread(producer, 25*i, 25);
    }

    for(auto i = 0; i < 4; i++){
        consumers[i].join();
        producers[i].join();
    }

    auto all_visited = true;
    for(auto i = 0; i < visited.size(); i++){
        all_visited &= visited[i];
    }
    REQUIRE(all_visited);

}

TEST_CASE( "Blocked thread wakes up when unblocked" ){

    BlockingQueue<int> bq;
    auto blocked = std::async( std::launch::async, [&]{bq.pop();} );
    bq.wake_up_blocked();
    auto status = blocked.wait_for(std::chrono::milliseconds(10));
    REQUIRE( status == std::future_status::ready );

}

