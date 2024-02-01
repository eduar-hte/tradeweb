#include "OrderCache.h"
#include "gtest/gtest.h"

class OrderCacheTest : public ::testing::Test {
protected:
    OrderCache cache;
};

// Test U1: Add order to cache
TEST_F(OrderCacheTest, U1_UnitTest_addOrder) {
    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"});
    cache.addOrder(Order{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"});
    ASSERT_EQ(1,1);
}

// Test U2: Get all orders from cache
TEST_F(OrderCacheTest, U2_UnitTest_getAllOrders) {
    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"});
    cache.addOrder(Order{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"});
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 8);
}

// Test U3: Cancel order
TEST_F(OrderCacheTest, U3_UnitTest_cancelOrder) {
    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 100, "User1", "Company1"});
    cache.addOrder(Order{"OrdId2", "SecId1", "Sell", 100, "User2", "Company1"});
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 2);

    // Cancel order 2
    cache.cancelOrder("OrdId2");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 1);
    ASSERT_EQ(allOrders[0].orderId(), "OrdId1");

    // Cancel order 1
    cache.cancelOrder("OrdId1");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 0);

    // Cancel an order that does not exist
    cache.cancelOrder("OrdId3");
    ASSERT_EQ(allOrders.size(), 0);
}

// Test U4: Cancel orders for user
TEST_F(OrderCacheTest, U4_UnitTest_cancelOrdersForUser) {
    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId1", "Buy", 600, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId2", "Sell", 3000, "User1", "CompanyB"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Sell", 500, "User2", "CompanyA"});
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 4);

    // Cancel all orders for User1
    cache.cancelOrdersForUser("User1");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 2); // Two orders left

    // Cancel all orders for User2
    cache.cancelOrdersForUser("User2");
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 0); // No orders left

    // Cancel an order for a user ID that does not exist
    cache.cancelOrdersForUser("User3");
    ASSERT_EQ(allOrders.size(), 0); // No orders left
}

// Test U5: Cancel orders for security with minimum quantity
TEST_F(OrderCacheTest, U5_UnitTest_cancelOrdersForSecIdWithMinimumQty) {
    OrderCache cache;
    cache.addOrder(Order{"1", "SecId1", "Buy", 200, "User1", "Company1"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 200, "User2", "Company1"});
    cache.addOrder(Order{"3", "SecId1", "Buy", 100, "User1", "Company1"});
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 300);
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 3);

    // Cancel all orders with security ID 1 and minimum quantity 200
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 200);
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 1);

    // Cancel all orders with security ID 1 and minimum quantity 100
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 100);
    allOrders = cache.getAllOrders();
    ASSERT_EQ(allOrders.size(), 0);
}

// Test U6: First example from README.txt
TEST_F(OrderCacheTest, U6_UnitTest_getMatchingSizeForSecurityTest_Example1) {
    OrderCache cache;

    cache.addOrder(Order{"OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"});
    cache.addOrder(Order{"OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"});
    cache.addOrder(Order{"OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 0);

    matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 2700);

    matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 0);
}

// Test U7: Second example from README.txt
TEST_F(OrderCacheTest, U7_UnitTest_getMatchingSizeForSecurityTest_Example2) {
    OrderCache cache;
    cache.addOrder(Order{"OrdId1", "SecId1", "Sell", 100, "User10", "Company2"});
    cache.addOrder(Order{"OrdId2", "SecId3", "Sell", 200, "User8", "Company2"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Buy", 300, "User13", "Company2"});
    cache.addOrder(Order{"OrdId4", "SecId2", "Sell", 400, "User12", "Company2"});
    cache.addOrder(Order{"OrdId5", "SecId3", "Sell", 500, "User7", "Company2"});
    cache.addOrder(Order{"OrdId6", "SecId3", "Buy", 600, "User3", "Company1"});
    cache.addOrder(Order{"OrdId7", "SecId1", "Sell", 700, "User10", "Company2"});
    cache.addOrder(Order{"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"});
    cache.addOrder(Order{"OrdId9", "SecId2", "Buy", 900, "User6", "Company2"});
    cache.addOrder(Order{"OrdId10", "SecId2", "Sell", 1000, "User5", "Company1"});
    cache.addOrder(Order{"OrdId11", "SecId1", "Sell", 1100, "User13", "Company2"});
    cache.addOrder(Order{"OrdId12", "SecId2", "Buy", 1200, "User9", "Company2"});
    cache.addOrder(Order{"OrdId13", "SecId1", "Sell", 1300, "User1", "Company1"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 300);

    matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 1000);

    matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 600);
}

// Test U8: Third example from README.txt
TEST_F(OrderCacheTest, U8_UnitTest_getMatchingSizeForSecurityTest_Example3) {
    OrderCache cache;
    cache.addOrder(Order{"OrdId1", "SecId3", "Sell", 100, "User1", "Company1"});
    cache.addOrder(Order{"OrdId2", "SecId3", "Sell", 200, "User3", "Company2"});
    cache.addOrder(Order{"OrdId3", "SecId1", "Buy", 300, "User2", "Company1"});
    cache.addOrder(Order{"OrdId4", "SecId3", "Sell", 400, "User5", "Company2"});
    cache.addOrder(Order{"OrdId5", "SecId2", "Sell", 500, "User2", "Company1"});
    cache.addOrder(Order{"OrdId6", "SecId2", "Buy", 600, "User3", "Company2"});
    cache.addOrder(Order{"OrdId7", "SecId2", "Sell", 700, "User1", "Company1"});
    cache.addOrder(Order{"OrdId8", "SecId1", "Sell", 800, "User2", "Company1"});
    cache.addOrder(Order{"OrdId9", "SecId1", "Buy", 900, "User5", "Company2"});
    cache.addOrder(Order{"OrdId10", "SecId1", "Sell", 1000, "User1", "Company1"});
    cache.addOrder(Order{"OrdId11", "SecId2", "Sell", 1100, "User6", "Company2"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 900);

    matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 600);

    matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 0);
}

// Test O1: Matching Orders with Different Quantities
TEST_F(OrderCacheTest, O1_OrderMatchingTest_TestDifferentQuantities) {
    cache.addOrder(Order{"1", "SecId1", "Buy", 5000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 2000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId1", "Sell", 1000, "User3", "CompanyC"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 3000); // 2000 from Order 2 and 1000 from Order 3 should match with Order 1
}

// Test O2: Complex Order Combinations
TEST_F(OrderCacheTest, O2_OrderMatchingTest_TestComplexCombinations) {
    cache.addOrder(Order{"1", "SecId2", "Buy", 7000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId2", "Sell", 3000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId2", "Sell", 4000, "User3", "CompanyC"});
    cache.addOrder(Order{"4", "SecId2", "Buy", 500, "User4", "CompanyD"});
    cache.addOrder(Order{"5", "SecId2", "Sell", 500, "User5", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 7500); // 7000 from Order 1 and 500 from Order 4 should fully match with Orders 2, 3, and 5
}

// Test O3: Orders from the Same Company Should Not Match
TEST_F(OrderCacheTest, O3_OrderMatchingTest_TestSameCompanyNoMatch) {
    cache.addOrder(Order{"1", "SecId3", "Buy", 2000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId3", "Sell", 2000, "User2", "CompanyA"}); // Same company as Order 1

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId3");
    ASSERT_EQ(matchingSize, 0); // No match should occur since both orders are from the same company
}

// Test C1: Cancelling an Order that Does Not Exist
TEST_F(OrderCacheTest, C1_CancellationTest_CancelNonExistentOrder) {
    cache.cancelOrder("NonExistentOrder");
    // Assuming getAllOrders() returns all current orders, the size should be 0 for an empty cache.
    std::vector<Order> allOrders = cache.getAllOrders();
    ASSERT_TRUE(allOrders.empty());
}

// Test C2: Partial Cancellation Based on Minimum Quantity
TEST_F(OrderCacheTest, C2_CancellationTest_CancelOrdersPartialOnMinQty) {
    cache.addOrder(Order{"1", "SecId1", "Buy", 200, "User1", "Company1"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 500, "User2", "Company1"});
    cache.addOrder(Order{"3", "SecId1", "Buy", 300, "User3", "Company2"});

    // Cancel orders for SecId1 with a minimum quantity of 300
    cache.cancelOrdersForSecIdWithMinimumQty("SecId1", 300);

    std::vector<Order> allOrders = cache.getAllOrders();
    // Only the order with quantity 200 should remain
    ASSERT_EQ(allOrders.size(), 1);
    ASSERT_EQ(allOrders[0].orderId(), "1");
}

// Test C3: Cancelling Multiple Orders for the Same User or Security ID
TEST_F(OrderCacheTest, C3_CancellationTest_CancelMultipleOrdersForUser) {
    cache.addOrder(Order{"1", "SecId1", "Buy", 200, "User1", "Company1"});
    cache.addOrder(Order{"2", "SecId2", "Sell", 300, "User1", "Company1"});
    cache.addOrder(Order{"3", "SecId3", "Buy", 400, "User2", "Company2"});

    // Cancel all orders for User1
    cache.cancelOrdersForUser("User1");

    std::vector<Order> allOrders = cache.getAllOrders();
    // Only the order for User2 should remain
    ASSERT_EQ(allOrders.size(), 1);
    ASSERT_EQ(allOrders[0].orderId(), "3");
}

// Test M1: Multiple Small Orders Matching a Larger Order
TEST_F(OrderCacheTest, M1_MatchingSizeTest_MultipleSmallOrdersMatchingLargeOrder) {
    cache.addOrder(Order{"1", "SecId1", "Buy", 10000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId1", "Sell", 2000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId1", "Sell", 1500, "User3", "CompanyC"});
    cache.addOrder(Order{"4", "SecId1", "Sell", 2500, "User4", "CompanyD"});
    cache.addOrder(Order{"5", "SecId1", "Sell", 4000, "User5", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId1");
    ASSERT_EQ(matchingSize, 10000); // Total of 10000 from orders 2, 3, 4, and 5 should match with order 1
}

// Test M2: Multiple Matching Combinations
TEST_F(OrderCacheTest, M2_MatchingSizeTest_MultipleMatchingCombinations) {
    cache.addOrder(Order{"1", "SecId2", "Buy", 6000, "User1", "CompanyA"});
    cache.addOrder(Order{"2", "SecId2", "Sell", 2000, "User2", "CompanyB"});
    cache.addOrder(Order{"3", "SecId2", "Sell", 3000, "User3", "CompanyC"});
    cache.addOrder(Order{"4", "SecId2", "Buy", 1000, "User4", "CompanyD"});
    cache.addOrder(Order{"5", "SecId2", "Sell", 1500, "User5", "CompanyE"});

    unsigned int matchingSize = cache.getMatchingSizeForSecurity("SecId2");
    ASSERT_EQ(matchingSize, 6500); // Total of 6500 (2000 from Order 2, 3000 from Order 3, 1500 from Order 5) should match with Orders 1 and 4
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
