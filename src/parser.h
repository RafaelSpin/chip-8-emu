#ifndef CHIP8_PARSER_8
#define CHIP8_PARSER_8

namespace emulation {
enum class Instruction {
  RETURN_SUBROUTINE,
  CLEAR_SCR,
  MACHINE_CALL_SUBROUTINE,
  JUMP,
  CALL_SUBROUTINE,
  SKIP_VX_EQUAL_N,
  SKIP_VX_NEQUAL_N,
  SKIP_VX_EQUAL_VY,
  ILOAD_NORMAL,
  IADD_NORMAL,
  LOAD_VY_VX,
  ORLOAD_VY_VX,
  ANDLOAD_VY_VX,
  XOR_LOAD_VY_VX,
  ADD_VY_VX,
  SUB_VY_VX,
  SHIFT_VY_RIGHT_VX,
  SUB_VX_VY,
  SHIFT_VY_LEFT_VX,
  SKIP_VX_NEQUAL_VY,
  ILOAD_INDEX,
  RANDOM_VX,
  JUMP_V0,
  DRAW_SPRITE,
  SKIP_IF_PRESSED,
  SKIP_IF_NPRESSED,
  STORE_DTIMER_VX,
  STORE_VX_KEY,
  SET_DTIMER_VX,
  SET_STIMER_VX,
  ADD_INDEX_VX,
  SET_INDEX_SPRITE,
  STORE_BIN_DEC_VX,
  STORE_VX_RANGE,
  LOAD_VX_RANGE,
};

using ushort = unsigned short;
class Parser {
  Instruction read_reg_binary_operations(ushort operation);
  Instruction read_F_operations(ushort operation);

public:
  Instruction read(ushort operation);
};
}; // namespace emulation

#endif
