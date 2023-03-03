/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long
 * as you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace paracl::containers {

template <typename T> class graph_node final : private std::vector<T> {
  T m_val;

  using vector = std::vector<T>;

public:
  using value_type = T;

  graph_node(const value_type val = value_type{}) : m_val{std::move(val)} {}

  operator value_type() { return m_val; }

  value_type value() const { return m_val; }

  void add_adj(const value_type &val) {
    if (std::find(begin(), end(), val) != end())
      throw std::logic_error{"Attempt to add existing edge into a oriented graph"};
    vector::push_back(val);
  }

  using vector::begin;
  using vector::end;

  using vector::cbegin;
  using vector::cend;

  using vector::empty;
  using vector::size;
};

template <typename node_t>
  requires std::derived_from<node_t, graph_node<typename node_t::value_type>>
class directed_graph;

template <typename graph_t>
  requires std::derived_from<graph_t, directed_graph<typename graph_t::node_type>>
class breadth_first_search;

template <typename node_t>
  requires std::derived_from<node_t, graph_node<typename node_t::value_type>>
class directed_graph {
public:
  using size_type = std::size_t;
  using value_type = typename node_t::value_type;
  using node_type = node_t;

private:
  std::unordered_map<value_type, node_t> m_adj_list;
  size_type m_edge_n = 0;

public:
  directed_graph(){};

  virtual ~directed_graph() {}

  virtual void insert_vertex(const value_type &val) { insert_vertex_base(val); }

  virtual void insert_edge(const value_type &first, const value_type &second) {
    if (!vertex_exists(first)) insert_vertex(first);
    if (!vertex_exists(second)) insert_vertex(second);
    insert_edge_base(first, second);
  }

  bool vertex_exists(const value_type &val) const { return m_adj_list.contains(val); }

  bool edge_exists(const value_type &first, const value_type &second) const {
    if (!(m_adj_list.contains(first) && m_adj_list.contains(second))) return false;
    auto &&list = m_adj_list.at(first);
    if (std::find(list.begin(), list.end(), second) == list.end()) return false;
    return true;
  }

  size_type number_of_edges() const { return m_edge_n; }

  size_type number_of_vertices() const { return m_adj_list.size(); }

  bool empty() const { return !number_of_vertices(); }

  size_type number_of_successors(const value_type &val) const {
    if (!m_adj_list.contains(val)) throw std::logic_error{"Attempt to get number of successors of non-existent vertex"};
    return m_adj_list[val].size();
  }

  // returns true if first is directly connected to second
  bool is_connected(const value_type &first, const value_type &second) const {
    if (!(m_adj_list.contains(first) && m_adj_list.contains(second)))
      throw std::logic_error{"Attempt to check for connection with non-existent vertex"};

    auto &&first_node = m_adj_list.at(first);
    auto &&found = std::find(first_node.begin(), first_node.end(), second);
    if (found == first_node.end()) return false;
    return true;
  }

  // returns true if second is reachable from the first
  bool is_reachable(const value_type &first, const value_type &second) const {
    breadth_first_search search{*this};
    return search(first, second);
  }

  auto find(const value_type &val) const { return m_adj_list.find(val); }
  auto contains(const value_type &val) const { return m_adj_list.contains(val); }

  auto begin() { return m_adj_list.begin(); }
  auto end() { return m_adj_list.end(); }
  auto begin() const { return m_adj_list.cbegin(); }
  auto end() const { return m_adj_list.cend(); }
  auto cbegin() const { return m_adj_list.cbegin(); }
  auto cend() const { return m_adj_list.cend(); }

protected:
  void insert_vertex_base(const value_type &val) {
    auto &&[iter, inserted] = m_adj_list.insert({val, {val}});
    if (!inserted) throw std::logic_error{"Attempt to insert existing vertex into a graph"};
  }

  // inserts vertices if they are not already at the DG
  void insert_edge_base(const value_type &vert1, const value_type &vert2) {
    if (!(m_adj_list.contains(vert1) && m_adj_list.contains(vert2)))
      throw std::logic_error{"Attempt to insert edge from or to non-existent node"};
    m_adj_list.at(vert1).add_adj(vert2);
    ++m_edge_n;
  }
};

template <typename T> using basic_directed_graph = directed_graph<graph_node<T>>;

template <typename graph_t>
  requires std::derived_from<graph_t, directed_graph<typename graph_t::node_type>>
class breadth_first_search final {
  const graph_t &m_graph;
  using value_type = typename graph_t::value_type;

  enum class color_t {
    E_WHITE,
    E_GRAY,
    E_BLACK
  };

  struct bfs_node {
    unsigned m_dist = std::numeric_limits<unsigned>::max();
    color_t m_color = color_t::E_WHITE;
    bfs_node *m_prev = nullptr;
  };

public:
  breadth_first_search(const graph_t &graph) : m_graph{graph} {}

  bool operator()(const value_type &root_val, const value_type &target) const {
    if (!m_graph.contains(root_val)) throw std::logic_error{"Non-existing vertex root in BFS"};
    if (!m_graph.contains(target)) return false;

    std::unordered_map<value_type, bfs_node> nodes;
    auto &&root_node = nodes.insert({root_val, {}}).first->second;
    root_node.m_color = color_t::E_GRAY;
    root_node.m_dist = 0;

    std::deque<value_type> que;
    que.push_back(root_val);
    while (!que.empty()) {
      auto &&curr = que.front();
      if (curr == target) return true;
      que.pop_front();
      auto &&curr_node = nodes.insert({curr, {}}).first->second;
      auto &&curr_graph_node = m_graph.find(curr)->second;
      for (auto &&adj : curr_graph_node) {
        auto &&adj_node = nodes.insert({adj, {}}).first->second;
        if (adj_node.m_color == color_t::E_WHITE) {
          adj_node.m_color = color_t::E_GRAY;
          adj_node.m_dist = curr_node.m_dist + 1;
          adj_node.m_prev = &curr_node;
          que.push_back(adj);
        }
      }
      curr_node.m_color = color_t::E_BLACK;
    }
    return false;
  }
};

template <typename graph_t>
  requires std::derived_from<graph_t, directed_graph<typename graph_t::node_type>>
std::vector<typename graph_t::value_type>
recursive_topo_sort(graph_t &graph, const typename graph_t::value_type &root_val) {
  using value_type = typename graph_t::value_type;
  enum class color_t {
    E_WHITE,
    E_GRAY,
    E_BLACK
  };

  struct bfs_node : public graph_t::node_type {
    int m_start = -1, m_finish = -1;
    color_t m_color = color_t::E_WHITE;
    bfs_node *m_prev = nullptr;

    bfs_node(const value_type &val) : graph_t::node_type{val} {}
  };

  int time = 0;
  std::vector<value_type> scheduled;
  std::unordered_map<value_type, bfs_node> nodes;

  for (auto &&val : graph)
    nodes.insert({val.first, val.first});

  std::function<void(value_type)> dfs_visit = [&time, &nodes, &dfs_visit, &graph, &scheduled](const value_type &val) {
    ++time;
    auto &&cur_node = nodes.at(val);
    cur_node.m_start = time;
    cur_node.m_color = color_t::E_GRAY;
    auto &&graph_node = graph.find(val)->second;
    for (auto &&adj : graph_node) {
      auto &&adj_node = nodes.at(adj);
      if (adj_node.m_color == color_t::E_WHITE) {
        adj_node.m_prev = &cur_node;
        dfs_visit(adj);
      }
    }
    cur_node.m_color = color_t::E_BLACK;
    scheduled.push_back(val);
    ++time;
    cur_node.m_finish = time;
  };

  for (auto &&val : graph) {
    if (nodes.at(val.first).m_color == color_t::E_WHITE) dfs_visit(val.first);
  }
  return std::vector(scheduled.rbegin(), scheduled.rend());
}

} // namespace paracl::containers