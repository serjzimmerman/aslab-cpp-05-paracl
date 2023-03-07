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

#include "callgraph.hpp"
#include "ezvis/ezvis.hpp"
#include "frontend/ast/ast_container.hpp"
#include "frontend/ast/ast_nodes/i_ast_node.hpp"
#include "frontend/error.hpp"
#include "frontend/symtab.hpp"
#include "frontend/types/types.hpp"
#include "function_table.hpp"
#include "functions_analytics.hpp"
#include "location.hpp"

#include <iostream>
#include <string_view>

namespace paracl::frontend {

class function_explorer final : public ezvis::visitor_base<ast::i_ast_node, function_explorer, void> {
private:
  std::vector<callgraph_value_type> m_function_stack;
  std::vector<error_report> *m_error_queue = nullptr;
  functions_analytics *m_analytics = nullptr;
  ast::ast_container *m_ast = nullptr;

  using to_visit = ast::tuple_all_nodes;
  void report_error(error_report report) { m_error_queue->push_back(std::move(report)); }

public:
  EZVIS_VISIT_CT(to_visit);

  void explore(const ast::binary_expression &ref) {
    apply(ref.right());
    apply(ref.left());
  }

  void explore(const ast::if_statement &ref) {
    apply(ref.cond());
    apply(ref.true_block());
    if (ref.else_block() != nullptr) apply(*ref.else_block());
  }

  void explore(const ast::statement_block &ref) {
    for (auto &statement : ref) {
      assert(statement);
      apply(*statement);
    }
  }

  void explore(const ast::assignment_statement &ref) { apply(ref.right()); }
  void explore(const ast::print_statement &ref) { apply(ref.expr()); }
  void explore(const ast::unary_expression &ref) { apply(ref.expr()); }

  void explore(const ast::while_statement &ref) {
    apply(ref.cond());
    apply(ref.block());
  }

  void explore(ast::function_definition &);

  void explore(const ast::function_definition_to_ptr_conv &ref) { apply(ref.definition()); }
  void explore(const ast::return_statement &ref) { apply(ref.expr()); }

  void explore(const ast::function_call &);
  void explore(const ast::i_ast_node &) {}

  EZVIS_VISIT_INVOKER(explore);

public:
  bool explore(ast::ast_container &ast, std::vector<error_report> &errors, functions_analytics &analytics) {
    errors.clear();
    m_function_stack.clear();
    m_ast = &ast;
    m_error_queue = &errors;
    m_analytics = &analytics;
    apply(*m_ast->get_root_ptr());
    return errors.empty();
  }
};
} // namespace paracl::frontend