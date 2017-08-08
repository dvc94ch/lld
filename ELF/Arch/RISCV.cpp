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
  RelExpr getRelExpr(uint32_t Type, const SymbolBody &S,
                     const uint8_t *Loc) const override;
};
} // namespace

static void or32le(uint8_t *Loc, uint32_t Val) {
  write32le(Loc, read32le(Loc) | Val);
}

void RISCV::relocateOne(uint8_t *Loc, uint32_t Type, uint64_t Val) const {
  switch (Type) {
  case R_RISCV_32:
    checkIntUInt<32>(Loc, Val, Type);
    write32le(Loc, Val);
    break;
  case R_RISCV_64:
    write64le(Loc, Val);
    break;
  case R_RISCV_HI20:
    checkUInt<32>(Loc, Val, Type);
    or32le(Loc, Val & 0xFFFFF000);
    break;
  case R_RISCV_LO12_I:
    checkUInt<32>(Loc, Val, Type);
    or32le(Loc, (Val & 0xFFF) << 20);
    break;
  case R_RISCV_LO12_S:
    checkUInt<32>(Loc, Val, Type);
    or32le(Loc, ((Val & 0xFE0) << 20) | ((Val & 0x1F) << 7));
    break;
  case R_RISCV_PCREL_HI20:
    checkInt<32>(Loc, Val, Type);
    or32le(Loc, (Val + 0x1000) & 0xFFFFF000);
    break;
  case R_RISCV_JAL:
    checkInt<21>(Loc, Val, Type);
    or32le(Loc, ((Val & 0x100000) << 11) | ((Val & 0x7FE)   << 20)
           |    ((Val & 0x800)    <<  9) | ((Val & 0xFF000) <<  0));
    break;
  case R_RISCV_BRANCH:
    checkInt<13>(Loc, Val, Type);
    or32le(Loc, ((Val & 0x1000) << 19) | ((Val & 0x7E0) << 20)
           |    ((Val & 0x1E)   <<  7) | ((Val & 0x800) >>  4));
    break;
  default:
    error(getErrorLocation(Loc) + "unrecognized reloc " + toString(Type));
  }
}

RelExpr RISCV::getRelExpr(uint32_t Type, const SymbolBody &S,
                          const uint8_t *Loc) const {
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
    error("unknown relocation type: " + toString(Type));
    return R_HINT;
  }
}

TargetInfo *elf::getRISCVTargetInfo() {
  static RISCV Target;
  return &Target;
}
