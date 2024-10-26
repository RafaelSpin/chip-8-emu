#include "parser.h"
#include <stdexcept>
using namespace emulation;

Instruction Parser::read_reg_binary_operations(ushort operation) {
  switch (operation & 0xF00F) {
  case 0x8000:
    return Instruction::LOAD_VY_VX;
  case 0x8001:
    return Instruction::ORLOAD_VY_VX;
  case 0x8002:
    return Instruction::ANDLOAD_VY_VX;
  case 0x8003:
    return Instruction::XOR_LOAD_VY_VX;
  case 0x8004:
    return Instruction::ADD_VY_VX;
  case 0x8005:
    return Instruction::SUB_VY_VX;
  case 0x8006:
    return Instruction::SHIFT_VY_RIGHT_VX;
  case 0x8007:
    return Instruction::SUB_VX_VY;
  case 0x800E:
    return Instruction::SHIFT_VY_LEFT_VX;
  default:
    throw std::runtime_error("Invalid operation \'" +
                             std::to_string(operation) + "\'");
  }
}

Instruction Parser::read_F_operations(ushort operation) {
  switch (operation & 0xF0FF) {
  case 0xF007:
    return Instruction::STORE_DTIMER_VX;
  case 0xF00A:
    return Instruction::STORE_VX_KEY;
  case 0xF015:
    return Instruction::SET_DTIMER_VX;
  case 0xF018:
    return Instruction::SET_STIMER_VX;
  case 0xF01E:
    return Instruction::ADD_INDEX_VX;
  case 0xF029:
    return Instruction::SET_INDEX_SPRITE;
  case 0xF033:
    return Instruction::STORE_BIN_DEC_VX;
  case 0xF055:
    return Instruction::STORE_VX_RANGE;
  case 0xF065:
    return Instruction::LOAD_VX_RANGE;
  default:
    throw std::runtime_error("Invalid operation \'" +
                             std::to_string(operation) + "\'");
  }
}

Instruction Parser::read(ushort operation) {
  switch (operation & 0xF000) {
  case 0xF000:
    return read_F_operations(operation);
  case 0xE000:
    operation &= 0x00FF;
    if (operation == 0x9E)
      return Instruction::SKIP_IF_PRESSED;
    else if (operation == 0xA1)
      return Instruction::SKIP_IF_NPRESSED;
  case 0xD000:
    return Instruction::DRAW_SPRITE;
  case 0xC000:
    return Instruction::RANDOM_VX;
  case 0xB000:
    return Instruction::JUMP_V0;
  case 0xA000:
    return Instruction::ILOAD_INDEX;
  case 0x9000:
    return Instruction::SKIP_VX_NEQUAL_VY;
  case 0x8000:
    return read_reg_binary_operations(operation);
  case 0x7000:
    return Instruction::IADD_NORMAL;
  case 0x6000:
    return Instruction::ILOAD_NORMAL;
  case 0x5000:
    return Instruction::SKIP_VX_EQUAL_VY;
  case 0x4000:
    return Instruction::SKIP_VX_NEQUAL_N;
  case 0x3000:
    return Instruction::SKIP_VX_EQUAL_N;
  case 0x2000:
    return Instruction::CALL_SUBROUTINE;
  case 0x1000:
    return Instruction::JUMP;
  case 0x0000:
    if (operation == 0x00EE)
      return Instruction::RETURN_SUBROUTINE;
    else if (operation == 0x00E0)
      return Instruction::CLEAR_SCR;
    else
      return Instruction::MACHINE_CALL_SUBROUTINE;
  default:
    throw std::runtime_error("Invalid operation \'" +
                             std::to_string(operation) + "\'");
  }
}
