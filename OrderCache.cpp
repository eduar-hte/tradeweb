// Todo: your implementation of the OrderCache...
#include "OrderCache.h"
#include <algorithm>

void OrderCache::addOrder(Order order)
{
  const std::lock_guard lock(_mutex); // write lock (exclusive access)

  const bool is_buy_order = order.side() == "Buy";

  OrderInfo order_info(order.qty());

  auto &asset_data = _orders_by_security[order.securityIdHash()];

  match_order(order, order_info, asset_data, is_buy_order);

  auto &orders = is_buy_order ? asset_data.buy_orders : asset_data.sell_orders;

  const auto order_id = order.orderIdHash();
  orders.insert({order_id, {std::move(order), std::move(order_info)}});
}

void OrderCache::cancelOrder(const std::string &orderId)
{
  const std::lock_guard lock(_mutex); // write lock (exclusive access)

  // NOTE: cancelOrder doesn't use cancelOrdersHelper in this version
  // because we can leverage that orders are stored in a map with their
  // orderIdHash as a key and find it more efficiently (than iterating
  // through all the orders)

  const auto order_id_hash = str_hash{}(orderId);

  auto cancel_helper = [order_id_hash](AssetData &asset_data, auto &orders, const bool is_buy_order)
  {
    auto it = orders.find(order_id_hash);
    if (it != orders.end())
    {
      unmatch_order(asset_data, it->second, is_buy_order);
      orders.erase(it);
      return true;
    }
    return false;
  };

  for (auto &x : _orders_by_security)
  {
    auto &asset_data = x.second;
    if (cancel_helper(asset_data, asset_data.buy_orders, true) == true || cancel_helper(asset_data, asset_data.sell_orders, false) == true)
    {
      // update matches because an order has been cancelled
      update_matches(asset_data);

      // order has already been found and cancelled, stop
      break;
    }
  }
}

void OrderCache::cancelOrdersForUser(const std::string &user)
{
  const std::lock_guard lock(_mutex); // write lock (exclusive access)

  const auto user_hash = str_hash{}(user);

  cancelOrdersHelper([user_hash](const auto &order)
                     { return order.userHash() == user_hash; });
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty)
{
  const std::lock_guard lock(_mutex); // write lock (exclusive access)

  const auto security_id_hash = str_hash{}(securityId);

  auto it = _orders_by_security.find(security_id_hash);
  if (it == _orders_by_security.end())
    return;

  cancelSecurityOrdersHelper(it->second,
                             [minQty](const auto &order)
                             { return order.qty() >= minQty; });
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string &securityId)
{
  const std::shared_lock lock(_mutex); // read lock (shared access)

  auto it = _orders_by_security.find(str_hash{}(securityId));
  return (it != _orders_by_security.end()) ? it->second.matching_size : 0;
}

std::vector<Order> OrderCache::getAllOrders() const
{
  const std::shared_lock lock(_mutex); // read lock (shared access)

  auto copy_orders = [](const auto &src, auto &dst)
  {
    std::transform(src.begin(), src.end(), std::back_inserter(dst), [](const auto &x)
                   { return x.second.first; });
  };

  std::vector<Order> orders;
  for (const auto &x : _orders_by_security)
  {
    copy_orders(x.second.buy_orders, orders);
    copy_orders(x.second.sell_orders, orders);
  }
  return orders;
}
