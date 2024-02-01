#include "OrderCache.h"
#include <iostream>
#include <iomanip>

void test()
{
    OrderCache cache;

    unsigned int orderNumber = 0;
    auto get_next_order_id = [&orderNumber]()
    { ++orderNumber; return "OrdId" + std::to_string(orderNumber); };

    for (auto i = 0; i != 10000; ++i)
    {
        cache.addOrder(Order{get_next_order_id(), "SecId1", "Buy", 1000, "User1", "CompanyA"});
        cache.addOrder(Order{get_next_order_id(), "SecId2", "Sell", 3000, "User2", "CompanyB"});
        cache.addOrder(Order{get_next_order_id(), "SecId1", "Sell", 500, "User3", "CompanyA"});
        cache.addOrder(Order{get_next_order_id(), "SecId2", "Buy", 600, "User4", "CompanyC"});
        cache.addOrder(Order{get_next_order_id(), "SecId2", "Buy", 100, "User5", "CompanyB"});
        cache.addOrder(Order{get_next_order_id(), "SecId3", "Buy", 1000, "User6", "CompanyD"});
        cache.addOrder(Order{get_next_order_id(), "SecId2", "Buy", 2000, "User7", "CompanyE"});
        cache.addOrder(Order{get_next_order_id(), "SecId2", "Sell", 5000, "User8", "CompanyE"});
    }
}

int main()
{
    auto benchmark = [](auto func)
    {
        const auto t1 = std::chrono::high_resolution_clock::now();

        func();

        const auto t2 = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> ms = t2 - t1;

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "benchmark time: " << ms.count() << " ms\n";
    };

    benchmark(test);

    return 0;
}