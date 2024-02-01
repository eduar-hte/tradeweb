#pragma once

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <mutex>

class Order
{

public:
  // do not alter signature of this constructor
  Order(
      const std::string &ordId,
      const std::string &secId,
      const std::string &side,
      const unsigned int qty,
      const std::string &user,
      const std::string &company)
      : m_orderId(ordId),
        m_securityId(secId),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_company(company) {}

  // do not alter these accessor methods
  std::string orderId() const { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const { return m_side; }
  std::string user() const { return m_user; }
  std::string company() const { return m_company; }
  unsigned int qty() const { return m_qty; }

private:
  // use the below to hold the order data
  // do not remove the these member variables
  std::string m_orderId;    // unique order id
  std::string m_securityId; // security identifier
  std::string m_side;       // side of the order, eg Buy or Sell
  unsigned int m_qty;       // qty for this order
  std::string m_user;       // user name who owns this order
  std::string m_company;    // company for user
};

// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed.
class OrderCacheInterface
{

public:
  // implement the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0;

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string &orderId) = 0;

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string &user) = 0;

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) = 0;

  // return the total qty that can match for the security id
  virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;
};

// Todo: Your implementation of the OrderCache...
class OrderCache : public OrderCacheInterface
{

public:
  void addOrder(Order order) override;

  void cancelOrder(const std::string &orderId) override;

  void cancelOrdersForUser(const std::string &user) override;

  void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override;

  unsigned int getMatchingSizeForSecurity(const std::string &securityId) override;

  std::vector<Order> getAllOrders() const override;

private:
  struct AssetData
  {
    std::list<Order> buy_orders;
    std::list<Order> sell_orders;
    std::unordered_map<std::string, unsigned int> total_buy_by_company;
    unsigned int matching_size = 0;
  };

  std::unordered_map<std::string, AssetData> _orders_by_security;
  mutable std::mutex _mutex;

  void update_matches(AssetData &asset_data)
  {
    // get a copy to update remaming qty as orders are matched
    auto total_buy_by_company = asset_data.total_buy_by_company;

    asset_data.matching_size = 0;

    for (const auto &order : asset_data.sell_orders)
    {
      auto pending_match = order.qty();

      for (auto &data : total_buy_by_company)
      {
        if (data.first != order.company() && data.second != 0)
        {
          const auto match = std::min(pending_match, data.second);
          pending_match -= match;
          data.second -= match;
          asset_data.matching_size += match;
        }

        if (pending_match == 0)
          break;
      }
    }
  }

  template <typename Pred>
  void cancelSecurityOrdersHelper(AssetData &asset_data, Pred pred)
  {
    auto cancel_helper = [](auto &orders, auto pred)
    {
      auto cancelled_orders = false;
      for (auto it = orders.begin(); it != orders.end();)
      {
        const auto ret = pred(*it);
        if (ret.first == true)
        {
          it = orders.erase(it);
          cancelled_orders = true;
        }
        else
          ++it;

        // check flag to indicate whether to stop cancelling orders
        // (used by cancel order by orderId for quick exit)
        if (ret.second == true)
          break;
      }
      return cancelled_orders;
    };

    // pred adapter to update total_buy_by_company if order is cancelled
    auto buy_orders_pred_adapter = [&](const auto &order)
    {
      const auto ret = pred(order);
      if (ret.first == true)
        asset_data.total_buy_by_company[order.company()] -= order.qty();
      return ret;
    };

    auto cancelled_orders = false;
    cancelled_orders |= cancel_helper(asset_data.buy_orders, buy_orders_pred_adapter);
    cancelled_orders |= cancel_helper(asset_data.sell_orders, pred);

    if (cancelled_orders == true)
      update_matches(asset_data);
  }

  template <typename Pred>
  void cancelOrdersHelper(Pred pred)
  {
    for (auto &x : _orders_by_security)
      cancelSecurityOrdersHelper(x.second, pred);
  }
};
