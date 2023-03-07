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

#include "ezvis/ezvis.hpp"
#include "location.hpp"

#include "frontend/analysis/augmented_ast.hpp"
#include "frontend/ast/ast_container.hpp"
#include "frontend/ast/ast_nodes.hpp"

#include "frontend/error.hpp"
#include "frontend/symtab.hpp"
#include "frontend/types/types.hpp"

#include <iostream>
#include <string_view>

namespace paracl::frontend {

class semantic_analyzer final : public ezvis::visitor_base<ast::i_ast_node, semantic_analyzer, void> {
private:
  symtab_stack m_scopes;

  ast::ast_container *m_ast;
  functions_analytics *m_functions;

  const types::builtin_types *m_types;
  std::vector<error_report> *m_error_queue;

private:
  enum class semantic_analysis_state {
    E_LVALUE,
    E_RVALUE,
    E_DEFAULT,
  } current_state = semantic_analysis_state::E_DEFAULT;
  bool m_in_function_body = false;

  void set_state(semantic_analysis_state s) { current_state = s; }
  void reset_state() { current_state = semantic_analysis_state::E_DEFAULT; }

  void report_error(std::string msg, location loc) {
    m_error_queue->push_back({
        error_kind{msg, loc}
    });
  }

  void report_error(error_report report) { m_error_queue->push_back(std::move(report)); }

  bool expect_type_eq(ast::i_expression &ref, types::i_type &rhs) {
    auto &&type = ref.m_type;
    if (!type || !(type->is_equal(rhs))) {
      report_error("Expression is not of type '" + rhs.to_string() + "'", ref.loc());
      return false;
    }

    return true;
  }

  bool expect_type_eq(ast::i_expression &ref, types::i_type *rhs) {
    if (!rhs) return false;
    return expect_type_eq(ref, *rhs);
  }

  using to_visit = ast::tuple_all_nodes;

public:
  EZVIS_VISIT_CT(to_visit);

  // clang-format off
  void analyze_node(ast::read_expression &);
  void analyze_node(ast::constant_expression &);
  // clang-format on

  void analyze_node(ast::assignment_statement &);
  void analyze_node(ast::binary_expression &);

  void analyze_node(ast::if_statement &);
  void analyze_node(ast::print_statement &);

  void analyze_node(ast::statement_block &);
  void analyze_node(ast::unary_expression &);
  bool analyze_node(ast::variable_expression &);
  void analyze_node(ast::while_statement &);
  void analyze_node(ast::error_node &);
  void analyze_node(ast::function_call &);
  void analyze_node(ast::function_definition &);
  void analyze_node(ast::function_definition_to_ptr_conv &);
  void analyze_node(ast::return_statement &) {}

  EZVIS_VISIT_INVOKER(analyze_node);

  bool analyze(
      ast::ast_container &ast, functions_analytics &functions, ast::i_ast_node &start,
      std::vector<error_report> &errors, bool in_main = false
  ) {
    errors.clear();

    // Set pointers to resources
    m_error_queue = &errors;
    m_ast = &ast;
    m_functions = &functions;
    m_types = &m_ast->builtin_types();

    // If we should visit the main scope, then we won't enter a function_definition node and set this flag ourselves.
    // This flag prevents the analyzer to go lower than 1 layer of functions;
    m_in_function_body = !in_main;

    apply(start);
    return errors.empty();
  }
};

} // namespace paracl::frontend