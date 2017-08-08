//===- RISCV.cpp ---------------------------------------------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Error.h"
#include "InputFiles.h"
#include "Symbols.h"
#include "Target.h"
#include "llvm/Object/ELF.h"
#include "llvm/Support/Endian.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;
using namespace lld;
using namespace lld::elf;

namespace {
class RISCV final : public TargetInfo {
public:
  void relocateOne(uint8_t *Loc, uint32_t Type, uint64_t Val) const override;
  RelExpr getRelExpr(uint32_t Type, const SymbolBody &S, const InputFile &File,
                     const uint8_t *Loc) const override;
};
} // namespace

static void applyRiscvReloc(uint8_t *Loc, uint64_t Val, uint32_t Mask) {
  write32le(Loc, (read32le(Loc) & ~Mask) | (Val & Mask));
}

void RISCV::relocateOne(uint8_t *Loc, uint32_t Type, uint64_t Val) const {
  switch (Type) {
  case R_RISCV_32:
    write32le(Loc, Val);
    break;
  case R_RISCV_64:
    write64le(Loc, Val);
    break;
  case R_RISCV_LO12_I:
    // I Instruction Format
    applyRiscvReloc(Loc, Val, 0xfff00000);
    break;
  case R_RISCV_LO12_S:
  case R_RISCV_BRANCH:
    // S / SB Instruction Formats
    applyRiscvReloc(Loc, Val, 0xFE000F80);
    break;
  case R_RISCV_HI20:
  case R_RISCV_PCREL_HI20:
  case R_RISCV_JAL:
    // U / UJ Instruction Formats
    applyRiscvReloc(Loc, Val, 0xfffff000);
    break;
  default:
    error(getErrorLocation(Loc) + "unrecognized reloc " + toString(Type));
  }
}

RelExpr RISCV::getRelExpr(uint32_t Type, const SymbolBody &S,
                          const InputFile &File, const uint8_t *Loc) const {
  switch (Type) {
  case R_RISCV_32:
  case R_RISCV_64:
  case R_RISCV_HI20:
  case R_RISCV_LO12_I:
  case R_RISCV_LO12_S:
    return R_ABS;
  case R_RISCV_PCREL_HI20:
  case R_RISCV_JAL:
  case R_RISCV_BRANCH:
    return R_PC;
  default:
    error(toString(&File) + ": unknown relocation type: " + toString(Type));
    return R_HINT;
  }
}

TargetInfo *elf::getRISCVTargetInfo() {
  static RISCV Target;
  return &Target;
}
