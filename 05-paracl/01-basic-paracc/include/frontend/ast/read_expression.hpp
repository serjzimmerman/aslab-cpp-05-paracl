/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "i_expression_node.hpp"
#include "i_statement_node.hpp"

#include "statement_block.hpp"

namespace paracl::frontend::ast {

class read_expression : public i_expression_node {
public:
  read_expression() = default;
};

static inline auto make_read_expression() { return i_expression_node_uptr{new read_expression{}}; }

} // namespace paracl::frontend::ast