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

#include "i_ast_node.hpp"
#include "location.hpp"

#include <string>

namespace paracl::frontend::ast {

class error_node final : public i_ast_node {
private:
  std::string m_error_message;

public:
  EZVIS_VISITABLE();
  error_node(const std::string &msg, location l) : i_ast_node{l}, m_error_message{msg} {};
  std::string error_msg() { return m_error_message; }
};

} // namespace paracl::frontend::ast