# Order Cache Implementation

## Simple/initial implementation

 * Directory simple contains an initial and more simple implementation of  `OrderCache` where only `OrderCache::getMatchingSizeForSecurity` is optimized to access the cached matching_size for each security.
 * This implementation uses a data structure to keep track of total buy qty by company (updated whenever a new Buy order for a security is added to the cache), as it assumes that the number of companies will be significantly smaller than the number of orders as orders grow.
 * However, matching size for each security is recalculated from scratch whenever an order is added/cancelled.

## Final implementation & submission

 * This implementation has improved performance when adding orders (compared to the initial implementation), by storing additional information (`OrderInfo`) about each order that indicates:
   * `unmatched`: how much of a buy/sell order is available for further matches.
   * `matches`: which orders have been matched against it.
     * this is used when an order is cancelled to revert matches against (`unmatch_order`).
 * Orders are stored in a `std::unordered_map` with their *security id* as key, and then into buy/sell `std::unordered_map` instances where their *order id* is the key.
   * This data structure is selected for amortized constant access when searching by their key.
   * In particular, grouping and storing orders by *security id* is intended to optimize access to its computed *matching size* by `getMatchingSizeForSecurity`.
     * This is secondarily useful for the performance of the `cancelOrdersForSecIdWithMinimumQty`, as only the orders for the required *security id* are reviewed.
   * Additionally, the buy/sell maps are used in the methods that cancel and unmatch orders.
     * Notice, however, that `cancelOrder` needs to iterate over the *orders by security id* data structure and then check in each buy/sell orders map to look for an order. 
 * Introduced hash values in each `Order` for: *order id*, *security id*, *user* & *company* for improved lookup/comparison using those members.
   * Assumes that `std::hash<std::string>` will not generate clashes on values used for these variables, which may need to be reviewed in real-world usage.
 * Leveraged `std::shared_mutex` to have read operations such as `getMatchingSizeForSecurity` & `getAllOrders` be allowed to concurrently access the cache data, and provide exclusive access to the cache to write operations, such as those that add or cancel orders.
 * Added additional test based on the first example from `README.txt` but that cancels one order (`OrdId8`), checks that the matching size has been correctly updated, and then re-adds it and checks the matching size again.
 * `benchmark.cpp` is a stand-alone binary to compare the performance of the initial and final `OrderCache` implementations.
   * To build, it requires building and linking with `OrderCache.cpp` too.

## Further work

 * To improve order matching performance, buy & sell order maps could be
 divided into two different containers, where those with available/unmatched quantities could be stored apart from those that have been fully matched, thus allowing for future matches to only iterate over those that are usable.
   * This would require introducing additional complexity to move orders from/to each of the available/used containers whenever an order is matched/unmatched, and thus left out of scope for this implementation.