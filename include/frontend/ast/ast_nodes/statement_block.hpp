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

#include "frontend/symtab.hpp"
#include "i_ast_node.hpp"

#include <cassert>
#include <vector>

namespace paracl::frontend::ast {

class statement_block final : public i_ast_node {
private:
  symtab m_symtab;
  std::vector<i_ast_node *> m_statements;

public:
  EZVIS_VISITABLE();

  statement_block() = default;
  statement_block(std::vector<i_ast_node *> vec, location l) : i_ast_node{l}, m_statements{vec} {}

  void append_statement(i_ast_node &statement) {
    const bool empty = m_statements.empty();
    m_statements.push_back(&statement);

    if (empty) {
      m_loc = statement.loc();
    } else {
      m_loc += statement.loc();
    }
  }

  auto size() const { return m_statements.size(); }
  auto begin() const { return m_statements.begin(); }
  auto end() const { return m_statements.end(); }

  symtab *symbol_table() { return &m_symtab; }
};

} // namespace paracl::frontend::ast