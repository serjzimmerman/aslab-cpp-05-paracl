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

#include "ezvis/ezvis.hpp"
#include "frontend/types/types.hpp"
#include "location.hpp"

namespace paracl::frontend::ast {

class i_ast_node : public ezvis::visitable_base<i_ast_node> {
protected:
  location m_loc;

protected:
  i_ast_node(location l = location{}) : m_loc{l} {}

public:
  EZVIS_VISITABLE();
  location loc() const { return m_loc; }

  virtual ~i_ast_node() {}
};

class i_expression : public i_ast_node {
public:
  types::shared_type m_type;

public:
  i_expression(location l = location{}, types::shared_type type = nullptr) : i_ast_node{l}, m_type{type} {}
  types::shared_type get_type() { return m_type; }

  std::string type_str() {
    if (!m_type.get()) return "<undetermined>";
    return m_type->to_string();
  }

  void set_type(types::shared_type type) { m_type = type; }
};

// Expresssions
class binary_expression;
class constant_expression;
class read_expression;
class assignment_statement;
class statement_block; // Is a statement as well as an expression, just like assignment
class unary_expression;
class variable_expression;
class function_call;
class function_definition_to_ptr_conv;

// Statements
class if_statement;
class print_statement;
class while_statement;
class function_definition;
class return_statement;

class error_node;

using tuple_all_nodes = std::tuple<
    assignment_statement, binary_expression, constant_expression, if_statement, print_statement, read_expression,
    statement_block, unary_expression, variable_expression, while_statement, error_node, function_definition,
    return_statement, function_call, function_definition_to_ptr_conv>;

using tuple_expression_nodes = std::tuple<
    assignment_statement, binary_expression, constant_expression, read_expression, statement_block, unary_expression,
    variable_expression, function_call, function_definition_to_ptr_conv>;

} // namespace paracl::frontend::ast