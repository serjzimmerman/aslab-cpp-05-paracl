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

#include "ast/visitor.hpp"
#include "utils/serialization.hpp"

#include <iostream>

namespace paracl::frontend::ast {

class ast_dumper final : public ezvis::visitor_base<i_ast_node, ast_dumper, void> {
private:
  using to_visit = tuple_ast_nodes;

private:
  std::ostream &m_os;

  static void print_declare_node(std::ostream &os, const i_ast_node &ref, std::string_view label) {
    os << "\tnode_0x" << std::hex << utils::pointer_to_uintptr(&ref) << " [label = \"" << label << "\" ];\n";
  }

  static void print_bind_node(std::ostream &os, const i_ast_node &parent, const i_ast_node &child,
                              std::string_view label = "") {
    os << "\tnode_0x" << std::hex << utils::pointer_to_uintptr(&parent) << " -> node_0x"
       << utils::pointer_to_uintptr(&child) << " [label = \"" << label << "\" ];\n";
  }

public:
  explicit ast_dumper(std::ostream &os) : m_os{os} {}

  EZVIS_VISIT(to_visit);

  void dump(assignment_statement &);
  void dump(binary_expression &);
  void dump(constant_expression &);
  void dump(if_statement &);
  void dump(print_statement &);
  void dump(read_expression &);
  void dump(statement_block &);
  void dump(unary_expression &);
  void dump(variable_expression &);
  void dump(while_statement &);
  void dump(error_node &);

  EZVIS_VISIT_INVOKER(dump);
};

inline void ast_dump(i_ast_node *node, std::ostream &os) {
  ast_dumper dumper{os};
  assert(node);

  os << "digraph abstract_syntax_tree {\n";
  dumper.apply(*node);
  os << "}\n";
}

} // namespace paracl::frontend::ast