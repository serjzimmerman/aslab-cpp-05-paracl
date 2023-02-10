/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tsimmerman.ss@phystech.edu>, <alex.rom23@mail.ru> wrote this file.  As long as you
 * retain this notice you can do whatever you want with this stuff. If we meet
 * some day, and you think this stuff is worth it, you can buy me a beer in
 * return.
 * ----------------------------------------------------------------------------
 */

#include "codegen.hpp"
#include "frontend/ast/ast_nodes.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace paracl::codegen {

namespace vm_instruction_set = bytecode_vm::instruction_set;
namespace vm_builder = bytecode_vm::builder;
namespace ast = frontend::ast;

void codegen_visitor::visit(ast::constant_expression *ptr) {
  assert(ptr);
  uint32_t index = lookup_or_insert_constant(ptr->value());
  m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::push_const_desc, index});
} // namespace frontend::ast::voidcodegen_visitor::visit(constant_expression*ptr)

void codegen_visitor::visit(ast::read_expression *ptr) {
  assert(ptr);
  m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::push_read_desc});
}

void codegen_visitor::visit(ast::error_node *) {
  throw std::logic_error{"Calling codegeneration on an ill-formed program"};
}

void codegen_visitor::visit(ast::variable_expression *ptr) {
  assert(ptr);
  auto index = m_symtab_stack.lookup_location(std::string{ptr->name()});
  m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::push_local_desc, index});
}

void codegen_visitor::visit(ast::print_statement *ptr) {
  assert(ptr);
  reset_currently_statement();
  ast_node_visit(*this, ptr->expr());
  m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::print_desc});
}

void codegen_visitor::visit(ast::assignment_statement *ptr) {
  assert(ptr);

  const bool emit_push = !is_currently_statement();
  ast_node_visit(*this, ptr->right());

  const auto last_it = std::prev(ptr->rend());
  for (auto start = ptr->rbegin(), finish = last_it; start != finish; ++start) {
    const auto left_index = m_symtab_stack.lookup_location(std::string{start->name()});
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::mov_local_desc, left_index});
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::push_local_desc, left_index});
  }

  // Last iteration:
  const auto left_index = m_symtab_stack.lookup_location(std::string{last_it->name()});
  m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::mov_local_desc, left_index});
  if (emit_push) {
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::push_local_desc, left_index});
  }
}

void codegen_visitor::visit(ast::binary_expression *ptr) {
  assert(ptr);
  reset_currently_statement();
  ast_node_visit(*this, ptr->left());

  reset_currently_statement();
  ast_node_visit(*this, ptr->right());

  using bin_op = ast::binary_operation;

  switch (ptr->op_type()) {
  case bin_op::E_BIN_OP_ADD:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::add_desc});
    break;
  case bin_op::E_BIN_OP_SUB:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::sub_desc});
    break;
  case bin_op::E_BIN_OP_MUL:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::mul_desc});
    break;
  case bin_op::E_BIN_OP_DIV:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::div_desc});
    break;
  case bin_op::E_BIN_OP_MOD:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::mod_desc});
    break;
  case bin_op::E_BIN_OP_EQ:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::cmp_eq_desc});
    break;
  case bin_op::E_BIN_OP_NE:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::cmp_ne_desc});
    break;
  case bin_op::E_BIN_OP_GT:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::cmp_gt_desc});
    break;
  case bin_op::E_BIN_OP_LS:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::cmp_ls_desc});
    break;
  case bin_op::E_BIN_OP_GE:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::cmp_ge_desc});
    break;
  case bin_op::E_BIN_OP_LE:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::cmp_le_desc});
    break;
  case bin_op::E_BIN_OP_AND:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::and_desc});
    break;
  case bin_op::E_BIN_OP_OR:
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::or_desc});
    break;
  }
}

void codegen_visitor::visit(ast::statement_block *ptr) {
  assert(ptr);
  m_symtab_stack.begin_scope();

  for (const auto &v : *ptr->symbol_table()) {
    m_symtab_stack.push_variable(v);
    m_builder.emit_operation(
        vm_builder::encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  for (auto &statement : *ptr) {
    set_currently_statement();
    ast_node_visit(*this, statement);
  }

  for (uint32_t i = 0; i < ptr->symbol_table()->size(); ++i) {
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::visit_if_no_else(ast::if_statement *ptr) {
  assert(ptr);
  reset_currently_statement();
  ast_node_visit(*this, ptr->cond());

  auto index_jmp_to_false_block =
      m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::jmp_false_desc, 0});

  set_currently_statement();
  ast_node_visit(*this, ptr->true_block());

  auto  jump_to_index = m_builder.current_loc();
  auto &to_relocate = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_false_block);

  std::get<0>(to_relocate.m_attr) = jump_to_index;
}

void codegen_visitor::visit_if_with_else(ast::if_statement *ptr) {
  assert(ptr);
  reset_currently_statement();
  ast_node_visit(*this, ptr->cond());

  auto index_jmp_to_false_block =
      m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::jmp_false_desc, 0});

  set_currently_statement();
  ast_node_visit(*this, ptr->true_block());
  auto index_jmp_to_after_true_block =
      m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::jmp_desc, 0});

  auto &to_relocate_else_jump = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_false_block);
  std::get<0>(to_relocate_else_jump.m_attr) = m_builder.current_loc();

  set_currently_statement();
  ast_node_visit(*this, ptr->else_block());

  auto &to_relocate_after_true_block = m_builder.get_as(vm_instruction_set::jmp_desc, index_jmp_to_after_true_block);
  std::get<0>(to_relocate_after_true_block.m_attr) = m_builder.current_loc();
}

void codegen_visitor::visit(ast::if_statement *ptr) {
  assert(ptr);
  m_symtab_stack.begin_scope();

  for (const auto &v : *ptr->control_block_symtab()) {
    m_symtab_stack.push_variable(v);
    m_builder.emit_operation(
        vm_builder::encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  if (!ptr->else_block()) {
    visit_if_no_else(ptr);
  } else {
    visit_if_with_else(ptr);
  }

  for (uint32_t i = 0; i < ptr->control_block_symtab()->size(); ++i) {
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::visit(ast::while_statement *ptr) {
  assert(ptr);
  m_symtab_stack.begin_scope();

  for (const auto &v : *ptr->symbol_table()) {
    m_symtab_stack.push_variable(v);
    m_builder.emit_operation(
        vm_builder::encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
  }

  auto while_location_start = m_builder.current_loc();
  reset_currently_statement();
  ast_node_visit(*this, ptr->cond());

  auto index_jmp_to_after_loop =
      m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::jmp_false_desc, 0});
  set_currently_statement();
  ast_node_visit(*this, ptr->block());
  m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::jmp_desc, while_location_start});

  auto &to_relocate_after_loop_jump = m_builder.get_as(vm_instruction_set::jmp_false_desc, index_jmp_to_after_loop);
  std::get<0>(to_relocate_after_loop_jump.m_attr) = m_builder.current_loc();

  for (uint32_t i = 0; i < ptr->symbol_table()->size(); ++i) {
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::pop_desc});
  }

  m_symtab_stack.end_scope();
}

void codegen_visitor::visit(ast::unary_expression *ptr) {
  assert(ptr);
  using unary_op = ast::unary_operation;

  reset_currently_statement();
  switch (ptr->op_type()) {
  case unary_op::E_UN_OP_NEG: {
    m_builder.emit_operation(
        vm_builder::encoded_instruction{vm_instruction_set::push_const_desc, lookup_or_insert_constant(0)});
    ast_node_visit(*this, ptr->expr());
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::sub_desc});
    break;
  }

  case unary_op::E_UN_OP_POS: {
    ast_node_visit(*this, ptr->expr()); /* Do nothing */
    break;
  }

  case unary_op::E_UN_OP_NOT: {
    ast_node_visit(*this, ptr->expr());
    m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::not_desc});
    break;
  }
  }
}

void codegen_visitor::set_currently_statement() { m_is_currently_statement = true; }
void codegen_visitor::reset_currently_statement() { m_is_currently_statement = false; }
bool codegen_visitor::is_currently_statement() const { return m_is_currently_statement; }

uint32_t codegen_visitor::lookup_or_insert_constant(int constant) {
  auto     found = m_constant_map.find(constant);
  uint32_t index;

  if (found == m_constant_map.end()) {
    index = m_constant_map.size();
    m_constant_map.insert({constant, index});
  }

  else {
    index = found->second;
  }

  return index;
}

paracl::bytecode_vm::decl_vm::chunk codegen_visitor::to_chunk() {
  // Last instruction is ret
  m_builder.emit_operation(vm_builder::encoded_instruction{vm_instruction_set::return_desc});

  auto ch = m_builder.to_chunk();

  std::vector<int> constants;
  constants.resize(m_constant_map.size());

  for (const auto &v : m_constant_map) {
    constants[v.second] = v.first;
  }

  ch.set_constant_pool(std::move(constants));
  return ch;
}

bytecode_vm::decl_vm::chunk generate_code(ast::i_ast_node *ptr) {
  assert(ptr);
  codegen_visitor generator;
  ast_node_visit(generator, ptr);
  return generator.to_chunk();
}

} // namespace paracl::codegen