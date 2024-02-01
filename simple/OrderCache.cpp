#include "OrderCache.h"

void OrderCache::addOrder(Order order)
{
  const std::lock_guard lock(_mutex);

  auto &data = _orders_by_security[order.securityId()];

  if (order.side() == "Buy")
  {
    // NOTE: update total_buy_by_company before moving order into buy_orders,
    // which invalidates the order object
    data.total_buy_by_company[order.company()] += order.qty();
    data.buy_orders.push_back(std::move(order));
  }
  else
    data.sell_orders.push_back(std::move(order));

  update_matches(data);
}

void OrderCache::cancelOrder(const std::string &orderId)
{
  const std::lock_guard lock(_mutex);

  auto pred = [orderId](const auto &order)
  {
    const auto match = order.orderId() == orderId;
    return std::make_pair(match, match);
  };

  cancelOrdersHelper(pred);
}

void OrderCache::cancelOrdersForUser(const std::string &user)
{
  const std::lock_guard lock(_mutex);

  auto pred = [user](const auto &order)
  {
    const auto match = order.user() == user;
    return std::make_pair(match, false);
  };

  cancelOrdersHelper(pred);
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty)
{
  const std::lock_guard lock(_mutex);

  auto it = _orders_by_security.find(securityId);
  if (it == _orders_by_security.end())
    return;

  auto pred = [securityId, minQty](const auto &order)
  {
    const auto match = order.securityId() == securityId && order.qty() >= minQty;
    return std::make_pair(match, false);
  };

  cancelSecurityOrdersHelper(it->second, pred);
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string &securityId)
{
  const std::lock_guard lock(_mutex);

  auto it = _orders_by_security.find(securityId);
  return (it != _orders_by_security.end()) ? it->second.matching_size : 0;
}

std::vector<Order> OrderCache::getAllOrders() const
{
  const std::lock_guard lock(_mutex);

  auto copy_elems = [](const auto &src, auto &dst)
  {
    std::copy(src.begin(), src.end(), std::back_inserter(dst));
  };

  std::vector<Order> orders;
  for (const auto &x : _orders_by_security)
  {
    copy_elems(x.second.buy_orders, orders);
    copy_elems(x.second.sell_orders, orders);
  }
  return orders;
}
