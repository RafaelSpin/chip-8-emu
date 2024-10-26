#include "executor.h"
#include "constants.h"
#include "parser.h"
#include "util.h"
#include <cmath>
#include <cstring>
#include <random>
#include <stdexcept>
using namespace emulation;

Executor::Executor(byte *memory) : memory{memory} {}

bool Executor::draw_sprite(ushort operation, byte vx, byte vy) {
  byte height = util::get_nth_nibble(operation, 0);
  byte xshift = nregister[vx], xrelshift = xshift % 8;
  // pixel position. the y axis needs to be flipped
  ushort pos =
      (screen::W_WIDTH / 8) * (screen::W_HEIGHT - nregister[vy]) + xshift / 8;
  bool changed = false;

  for (ushort i = 0; i < height; i++) {
    byte yshift = nregister[vy] + i;
    if ((xshift >= screen::W_WIDTH) || (yshift >= screen::W_HEIGHT))
      break;

    pos -= screen::W_WIDTH / 8;
    // extract each byte to paint from sprite, big endian order
    byte pixel = memory[iregister + i];
    // shift and reposition the row bits according to starting position
    byte newpixel = pixel >> xrelshift;
    byte fornext = pixel << (8 - xrelshift);

    if (!changed && ((pixels[pos] & newpixel) || (pixels[pos] & fornext)))
      changed = true;
    pixels[pos] ^= newpixel;
    pixels[pos + 1] ^= fornext;
  }
  return changed;
}

int Executor::execute(ushort operation, Instruction instruction,
                      ushort lastKey) {
  byte vx = util::get_nth_nibble(operation, 2),
       vy = util::get_nth_nibble(operation, 1);

  bool increment = true;
  short op;
  static byte waitingKey = 0x00;
  switch (instruction) {
  case Instruction::RETURN_SUBROUTINE:
    pc = stack[--sp];
    break;
  case Instruction::CLEAR_SCR:
    memset(pixels, 0, screen::W_WIDTH * screen::W_HEIGHT / 8);
    break;
  case Instruction::MACHINE_CALL_SUBROUTINE:
    break; // operation 0NNN is ignored in modern interpreters
  case Instruction::JUMP:
    pc = util::get_addr(operation);
    increment = false;
    break;
  case Instruction::CALL_SUBROUTINE:
    stack[sp++] = pc;
    pc = util::get_addr(operation);
    increment = false;
    break;
  case Instruction::SKIP_VX_EQUAL_N:
    if (nregister[vx] == util::get_nth_byte(operation, 0))
      increment_pc();
    break;
  case Instruction::SKIP_VX_NEQUAL_N:
    if (nregister[vx] != util::get_nth_byte(operation, 0))
      increment_pc();
    break;
  case Instruction::SKIP_VX_EQUAL_VY:
    if (nregister[vx] == nregister[vy])
      increment_pc();
    break;
  case Instruction::ILOAD_NORMAL:
    nregister[vx] = util::get_nth_byte(operation, 0);
    break;
  case Instruction::IADD_NORMAL:
    nregister[vx] += util::get_nth_byte(operation, 0);
    break;
  case Instruction::LOAD_VY_VX:
    nregister[vx] = nregister[vy];
    break;
  case Instruction::ORLOAD_VY_VX:
    nregister[vx] |= nregister[vy];
    break;
  case Instruction::ANDLOAD_VY_VX:
    nregister[vx] &= nregister[vy];
    break;
  case Instruction::XOR_LOAD_VY_VX:
    nregister[vx] ^= nregister[vy];
    break;
  case Instruction::ADD_VY_VX:
    op = nregister[vx] + nregister[vy];
    nregister[vx] = (byte)op;
    if (op > 0xFF)
      nregister[15] = 0x01;
    else
      nregister[15] = 0x00;
    break;
  case Instruction::SUB_VY_VX:
    op = nregister[vx] - nregister[vy];
    nregister[vx] = (byte)op;
    if (op < 0x0)
      nregister[15] = 0x00;
    else
      nregister[15] = 0x01;
    break;
  case Instruction::SHIFT_VY_RIGHT_VX:
    op = nregister[vy] & 0x01;
    nregister[vx] = nregister[vy] >> 1;
    nregister[15] = op;
    break;
  case Instruction::SUB_VX_VY:
    op = nregister[vy] - nregister[vx];
    nregister[vx] = (byte)op;
    if (op < 0x0)
      nregister[15] = 0x00;
    else
      nregister[15] = 0x01;
    break;
  case Instruction::SHIFT_VY_LEFT_VX:
    op = (nregister[vy] >> 7) & 0x01;
    nregister[vx] = nregister[vy] << 1;
    nregister[15] = op;
    break;
  case Instruction::SKIP_VX_NEQUAL_VY:
    if (nregister[vx] != nregister[vy])
      increment_pc();
    break;
  case Instruction::ILOAD_INDEX:
    iregister = util::get_addr(operation);
    break;
  case Instruction::RANDOM_VX:
    nregister[vx] = util::random_byte() & util::get_nth_byte(operation, 0);
    break;
  case Instruction::JUMP_V0:
    pc = util::get_addr(operation) + nregister[0];
    increment = false;
    break;
  case Instruction::DRAW_SPRITE:
    if (draw_sprite(operation, vx, vy))
      nregister[15] = 0x01;
    else
      nregister[15] = 0x00;
    break;
  case Instruction::SKIP_IF_PRESSED:
    if (lastKey & (1 << nregister[vx]))
      increment_pc();
    break;
  case Instruction::SKIP_IF_NPRESSED:
    if (!(lastKey & (1 << nregister[vx])))
      increment_pc();
    break;
  case Instruction::ADD_INDEX_VX:
    iregister += nregister[vx];
    break;
  case Instruction::SET_INDEX_SPRITE:
    iregister = FONT_START_ADDR + FONT_HEIGHT * nregister[vx];
    break;
  case Instruction::STORE_BIN_DEC_VX:
    for (int i = 0; i < 3; i++)
      memory[iregister + i] = (nregister[vx] / ((int)pow(10, 2 - i))) % 10;
    break;
  case Instruction::STORE_DTIMER_VX:
    nregister[vx] = delayTimer;
    break;
  case Instruction::STORE_VX_KEY:
    if (lastKey == 0x00)
      increment = false;
    else if (waitingKey == 0x00) {
      nregister[vx] = waitingKey = (byte)std::log2(lastKey);
      increment = false;
    } else if (lastKey & (1 << waitingKey))
      increment = false;
    else
      waitingKey = 0x00;
    break;
  case Instruction::SET_DTIMER_VX:
    delayTimer = nregister[vx];
    break;
  case Instruction::SET_STIMER_VX:
    soundTimer = nregister[vx];
    break;
  case Instruction::STORE_VX_RANGE:
    for (int i = 0; i <= vx; i++)
      memory[iregister++] = nregister[i];
    break;
  case Instruction::LOAD_VX_RANGE:
    for (int i = 0; i <= vx; i++)
      nregister[i] = memory[iregister++];
    break;
  default:
    throw std::runtime_error("Invalid operation \'" +
                             std::to_string(operation) + "\'");
  }

  if (increment)
    increment_pc();
  return 0;
}
