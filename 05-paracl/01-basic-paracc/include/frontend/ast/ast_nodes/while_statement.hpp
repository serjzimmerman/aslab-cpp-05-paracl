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

#include "frontend/symtab.hpp"
#include "i_ast_node.hpp"
#include "statement_block.hpp"

namespace paracl::frontend::ast {

class while_statement : public i_ast_node {
  symtab          m_symtab;
  i_ast_node_uptr m_condition;
  i_ast_node_uptr m_block;

public:
  while_statement(i_ast_node_uptr cond, i_ast_node_uptr block, location l)
      : i_ast_node{l}, m_condition{std::move(cond)}, m_block{std::move(block)} {}

  void accept(i_ast_visitor &visitor) { visitor.visit(this); }

  i_ast_node *cond() { return m_condition.get(); }
  i_ast_node *block() { return m_block.get(); }

  symtab *symbol_table() { return &m_symtab; }
};

static inline i_ast_node_uptr make_while_statement(i_ast_node_uptr cond, i_ast_node_uptr block, location l) {
  return std::make_unique<while_statement>(std::move(cond), std::move(block), l);
}

} // namespace paracl::frontend::ast