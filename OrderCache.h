#pragma once

#include <string>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <shared_mutex>
#include <functional>
#include <utility>
#include <cassert>

using str_hash = std::hash<std::string>;

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
        m_orderIdHash(str_hash{}(ordId)),
        m_securityId(secId),
        m_securityIdHash(str_hash{}(secId)),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_userHash(str_hash{}(user)),
        m_company(company),
        m_companyHash(str_hash{}(company)) {}

  // do not alter these accessor methods
  std::string orderId() const { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const { return m_side; }
  std::string user() const { return m_user; }
  std::string company() const { return m_company; }
  unsigned int qty() const { return m_qty; }

  size_t orderIdHash() const { return m_orderIdHash; }
  size_t securityIdHash() const { return m_securityIdHash; }
  size_t userHash() const { return m_userHash; }
  size_t companyHash() const { return m_companyHash; }

private:
  // use the below to hold the order data
  // do not remove the these member variables
  std::string m_orderId;    // unique order id
  std::string m_securityId; // security identifier
  std::string m_side;       // side of the order, eg Buy or Sell
  unsigned int m_qty;       // qty for this order
  std::string m_user;       // user name who owns this order
  std::string m_company;    // company for user

  const size_t m_orderIdHash;
  const size_t m_securityIdHash;
  const size_t m_userHash;
  const size_t m_companyHash;
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
  struct pair_hash
  {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
      const auto h1 = std::hash<T1>{}(p.first);
      const auto h2 = std::hash<T2>{}(p.second);
      return h1 ^ (h2 << 1);
    }
  };

  struct OrderInfo
  {
    OrderInfo(unsigned int qty) : unmatched(qty) {}
    std::unordered_set<size_t> order_matches;
    unsigned int unmatched;
  };

  struct AssetData
  {
    using OrderData = std::pair<Order, OrderInfo>;
    using OrdersMap = std::unordered_map<size_t, OrderData>;

    OrdersMap buy_orders;
    OrdersMap sell_orders;

    using MatchedOrderPair = std::pair<size_t, size_t>;

    std::unordered_map<MatchedOrderPair, unsigned int, pair_hash> matches;
    unsigned int matching_size = 0;
  };

  std::unordered_map<size_t, AssetData> _orders_by_security;
  mutable std::shared_mutex _mutex;

  static inline AssetData::MatchedOrderPair get_matched_order_pair(const size_t order_id, const size_t other_side_order_id, const bool is_buy_order)
  {
    if (is_buy_order == true)
      return {other_side_order_id, order_id};
    else
      return {order_id, other_side_order_id};
  }

  static inline void match_order(const Order &order, OrderInfo &order_info, AssetData &asset_data, const bool is_buy_order)
  {
    // if the order has already been fully matched, stop
    if (order_info.unmatched == 0)
      return;

    auto &orders = is_buy_order == true ? asset_data.sell_orders : asset_data.buy_orders;

    for (auto &order_elem : orders)
    {
      auto &other_side_order_data = order_elem.second;
      if (other_side_order_data.second.unmatched != 0 && other_side_order_data.first.companyHash() != order.companyHash())
      {
        // determine how much we can match from both orders and remove
        // from pending/available qty for further matches
        const auto match = std::min(order_info.unmatched, other_side_order_data.second.unmatched);
        order_info.unmatched -= match;
        other_side_order_data.second.unmatched -= match;
        asset_data.matching_size += match;

        // store matching info for order cancellation & unmatching process
        const auto order_id = order.orderIdHash();
        const auto other_side_order_id = other_side_order_data.first.orderIdHash();

        order_info.order_matches.insert(other_side_order_id);
        other_side_order_data.second.order_matches.insert(order_id);

        asset_data.matches.insert({get_matched_order_pair(order_id, other_side_order_id, is_buy_order), match});

        // if the order has already been fully matched, stop
        if (order_info.unmatched == 0)
          break;
      }
    }
  };

  static inline void unmatch_order(AssetData &asset_data, AssetData::OrderData &order_data, const bool is_buy_order)
  {
    const auto order_id = order_data.first.orderIdHash();
    auto &other_side_orders = is_buy_order == true ? asset_data.sell_orders : asset_data.buy_orders;

    for (const auto other_side_order_id : order_data.second.order_matches)
    {
      auto match_info_it = asset_data.matches.find(get_matched_order_pair(order_id, other_side_order_id, is_buy_order));
      assert(match_info_it != asset_data.matches.end());

      auto other_side_order_it = other_side_orders.find(other_side_order_id);
      assert(other_side_order_it != other_side_orders.end());

      auto &other_side_order_info = other_side_order_it->second.second;

      // restore previously matched qty
      other_side_order_info.unmatched += match_info_it->second;

      // remove unmatched order from other side's matched orders
      other_side_order_info.order_matches.erase(order_id);

      // decrease matching size by matched qty
      asset_data.matching_size -= match_info_it->second;

      // remove match info entry
      asset_data.matches.erase(match_info_it);
    }
  };

  inline void update_matches(AssetData &asset_data)
  {
    for (auto &sell_order_data : asset_data.sell_orders)
      match_order(sell_order_data.second.first, sell_order_data.second.second, asset_data, false);
  }

  template <typename Pred>
  inline void cancelSecurityOrdersHelper(AssetData &asset_data, Pred pred)
  {
    auto cancel_helper = [&asset_data](auto &orders, auto pred, const bool is_buy_order)
    {
      auto cancelled_orders = false;
      for (auto it = orders.begin(); it != orders.end();)
      {
        const auto ret = pred(it->second.first);
        if (ret == true)
        {
          unmatch_order(asset_data, it->second, is_buy_order);
          it = orders.erase(it);
          cancelled_orders = true;
        }
        else
          ++it;
      }
      return cancelled_orders;
    };

    auto cancelled_orders = false;
    cancelled_orders |= cancel_helper(asset_data.buy_orders, pred, true);
    cancelled_orders |= cancel_helper(asset_data.sell_orders, pred, false);

    if (cancelled_orders == true)
      update_matches(asset_data);
  }

  template <typename Pred>
  inline void cancelOrdersHelper(Pred pred)
  {
    for (auto &x : _orders_by_security)
      cancelSecurityOrdersHelper(x.second, pred);
  }
};
