/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "frontend/ast/ast_nodes/i_ast_node.hpp"
#include "visitor.hpp"

#include <cassert>

namespace paracl::frontend::ast {

class ast_container;

class ast_copier final : public i_ast_visitor {
private:
  ast_container &m_container;
  i_ast_node    *m_ret_node = nullptr;

  void return_node(i_ast_node *ptr) {
    assert(ptr);
    m_ret_node = ptr;
  }

  template <typename T> T *copy_subtree(T *node) {
    ast_node_visit(*this, node);
    return get_return_as<T>();
  }

public:
  ast_copier(ast_container &container) : m_container{container} {}

  void visit(assignment_statement *ptr) override;
  void visit(binary_expression *ptr) override;
  void visit(constant_expression *ptr) override;
  void visit(if_statement *) override;
  void visit(print_statement *ptr) override;
  void visit(read_expression *ptr) override;
  void visit(statement_block *ptr) override;
  void visit(unary_expression *) override;
  void visit(variable_expression *) override;
  void visit(while_statement *) override;
  void visit(error_node *) override;

  i_ast_node              *get_return() { return m_ret_node; }
  template <typename T> T *get_return_as() { return static_cast<T *>(m_ret_node); }
};

i_ast_node *ast_copy(i_ast_node *node, ast_container &container);

} // namespace paracl::frontend::ast

#include "ast_container.hpp"