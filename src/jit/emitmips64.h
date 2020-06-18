// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

// Copyright (c) Loongson Technology. All rights reserved.

#if defined(_TARGET_MIPS64_)

// The MIPS64 instructions are all 32 bits in size.
// we use an unsigned int to hold the encoded instructions.
// This typedef defines the type that we use to hold encoded instructions.
//
typedef unsigned int code_t;

static bool strictMIPSAsm;

/************************************************************************/
/*         Routines that compute the size of / encode instructions      */
/************************************************************************/

struct CnsVal
{
    ssize_t cnsVal;
    bool    cnsReloc;
};

#ifdef DEBUG

/************************************************************************/
/*             Debug-only routines to display instructions              */
/************************************************************************/

const char* emitFPregName(unsigned reg, bool varName = true);
const char* emitVectorRegName(regNumber reg);

//NOTE: At least 32bytes within dst.
void emitDisInsName(code_t code, const BYTE* dst);

#if 0
//NOTE: these function are not used for mips64's disassemble.
void emitDispInst(instruction ins);
void emitDispReloc(int value, bool addComma);
void emitDispImm(ssize_t imm, bool addComma, bool alwaysHex = false);
void emitDispFloatZero();
void emitDispFloatImm(ssize_t imm8);
void emitDispBarrier(insBarrier barrier);
void emitDispReg(regNumber reg, emitAttr attr, bool addComma);
void emitDispVectorReg(regNumber reg, insOpts opt, bool addComma);
void emitDispVectorRegIndex(regNumber reg, emitAttr elemsize, ssize_t index, bool addComma);
void emitDispArrangement(insOpts opt);
void emitDispAddrRI(regNumber reg, insOpts opt, ssize_t imm);
#endif

void emitDispIns(instrDesc* id,
                 bool       isNew,
                 bool       doffs,
                 bool       asmfm,
                 unsigned   offs  = 0,
                 BYTE*      pCode = 0,
                 size_t     sz    = 0,
                 insGroup*  ig    = NULL);
#endif // DEBUG

/************************************************************************/
/*  Private members that deal with target-dependent instr. descriptors  */
/************************************************************************/

private:
instrDesc* emitNewInstrAmd(emitAttr attr, int dsp);
instrDesc* emitNewInstrAmdCns(emitAttr attr, int dsp, int cns);

instrDesc* emitNewInstrCallDir(int              argCnt,
                               VARSET_VALARG_TP GCvars,
                               regMaskTP        gcrefRegs,
                               regMaskTP        byrefRegs,
                               emitAttr         retSize
                               MULTIREG_HAS_SECOND_GC_RET_ONLY_ARG(emitAttr secondRetSize));

instrDesc* emitNewInstrCallInd(int              argCnt,
                               ssize_t          disp,
                               VARSET_VALARG_TP GCvars,
                               regMaskTP        gcrefRegs,
                               regMaskTP        byrefRegs,
                               emitAttr         retSize
                               MULTIREG_HAS_SECOND_GC_RET_ONLY_ARG(emitAttr secondRetSize));

void emitGetInsCns(instrDesc* id, CnsVal* cv);
ssize_t emitGetInsAmdCns(instrDesc* id, CnsVal* cv);
void emitGetInsDcmCns(instrDesc* id, CnsVal* cv);
ssize_t emitGetInsAmdAny(instrDesc* id);

/************************************************************************/
/*               Private helpers for instruction output                 */
/************************************************************************/

private:
bool emitInsIsCompare(instruction ins);
bool emitInsIsLoad(instruction ins);
bool emitInsIsStore(instruction ins);
bool emitInsIsLoadOrStore(instruction ins);
emitAttr emitInsAdjustLoadStoreAttr(instruction ins, emitAttr attr);
emitAttr emitInsTargetRegSize(instrDesc* id);
emitAttr emitInsLoadStoreSize(instrDesc* id);

emitter::insFormat emitInsFormat(instruction ins);
emitter::code_t emitInsCode(instruction ins /*, insFormat fmt*/);

// Generate code for a load or store operation and handle the case of contained GT_LEA op1 with [base + index<<scale +
// offset]
void emitInsLoadStoreOp(instruction ins, emitAttr attr, regNumber dataReg, GenTreeIndir* indir);

//  Emit the 32-bit MIPS64 instruction 'code' into the 'dst'  buffer
static unsigned emitOutput_Instr(BYTE* dst, code_t code);

////FIXME for MIPS: should delete these !

// A helper method to return the natural scale for an EA 'size'
static unsigned NaturalScale_helper(emitAttr size);

// A helper method to perform a Rotate-Right shift operation
static UINT64 ROR_helper(UINT64 value, unsigned sh, unsigned width);

// A helper method to perform a 'NOT' bitwise complement operation
static UINT64 NOT_helper(UINT64 value, unsigned width);

// A helper method to perform a bit Replicate operation
static UINT64 Replicate_helper(UINT64 value, unsigned width, emitAttr size);

#if 1
/************************************************************************
*
* This union is used to to encode/decode the special MIPS64 immediate values
* that is listed as imm(N,r,s) and referred to as 'bitmask immediate'
*/

union bitMaskImm {
    struct
    {
        unsigned immS : 6; // bits 0..5
        unsigned immR : 6; // bits 6..11
        unsigned immN : 1; // bits 12
    };
    unsigned immNRS; // concat N:R:S forming a 13-bit unsigned immediate
};

/************************************************************************
*
*  Convert between a 64-bit immediate and its 'bitmask immediate'
*   representation imm(i16,hw)
*/

static emitter::bitMaskImm emitEncodeBitMaskImm(INT64 imm, emitAttr size);

static INT64 emitDecodeBitMaskImm(const emitter::bitMaskImm bmImm, emitAttr size);

/************************************************************************
*
* This union is used to to encode/decode the special MIPS64 immediate values
* that is listed as imm(i16,hw) and referred to as 'halfword immediate'
*/

union halfwordImm {
    struct
    {
        unsigned immVal : 16; // bits  0..15
        unsigned immHW : 2;   // bits 16..17
    };
    unsigned immHWVal; // concat HW:Val forming a 18-bit unsigned immediate
};

/************************************************************************
*
*  Convert between a 64-bit immediate and its 'halfword immediate'
*   representation imm(i16,hw)
*/

static emitter::halfwordImm emitEncodeHalfwordImm(INT64 imm, emitAttr size);

static INT64 emitDecodeHalfwordImm(const emitter::halfwordImm hwImm, emitAttr size);

/************************************************************************
*
* This union is used to encode/decode the special MIPS64 immediate values
* that is listed as imm(i16,by) and referred to as 'byteShifted immediate'
*/

union byteShiftedImm {
    struct
    {
        unsigned immVal : 8;  // bits  0..7
        unsigned immBY : 2;   // bits  8..9
        unsigned immOnes : 1; // bit   10
    };
    unsigned immBSVal; // concat Ones:BY:Val forming a 10-bit unsigned immediate
};

/************************************************************************
*
*  Convert between a 16/32-bit immediate and its 'byteShifted immediate'
*   representation imm(i8,by)
*/

static emitter::byteShiftedImm emitEncodeByteShiftedImm(INT64 imm, emitAttr size, bool allow_MSL);

static INT32 emitDecodeByteShiftedImm(const emitter::byteShiftedImm bsImm, emitAttr size);

/************************************************************************
*
* This union is used to to encode/decode the special MIPS64 immediate values
* that are use for FMOV immediate and referred to as 'float 8-bit immediate'
*/

union floatImm8 {
    struct
    {
        unsigned immMant : 4; // bits 0..3
        unsigned immExp : 3;  // bits 4..6
        unsigned immSign : 1; // bits 7
    };
    unsigned immFPIVal; // concat Sign:Exp:Mant forming an 8-bit unsigned immediate
};

/************************************************************************
*
*  Convert between a double and its 'float 8-bit immediate' representation
*/

static emitter::floatImm8 emitEncodeFloatImm8(double immDbl);

static double emitDecodeFloatImm8(const emitter::floatImm8 fpImm);

/************************************************************************
*
*  This union is used to to encode/decode the cond, nzcv and imm5 values for
*   instructions that use them in the small constant immediate field
*/

union condFlagsImm {
    struct
    {
        //insCond   cond : 4;  // bits  0..3
        //insCflags flags : 4; // bits  4..7
        unsigned  imm5 : 5;  // bits  8..12
    };
    unsigned immCFVal; // concat imm5:flags:cond forming an 13-bit unsigned immediate
};

// Returns an encoding for the specified register used in the 'Rt' position
static code_t insEncodeReg_Rt(regNumber reg);

// Returns an encoding for the specified register used in the 'Rs' position
static code_t insEncodeReg_Rs(regNumber reg);

// Returns an encoding for the specified register used in the 'Rd' position
static code_t insEncodeReg_Rd(regNumber reg);

// Returns an encoding for the specified register used in the 'Rm' position
static code_t insEncodeReg_Rm(regNumber reg);

// Returns an encoding for the specified register used in the 'Ra' position
static code_t insEncodeReg_Ra(regNumber reg);

// Returns an encoding for the specified register used in the 'Vd' position
static code_t insEncodeReg_Vd(regNumber reg);

// Returns an encoding for the specified register used in the 'Vt' position
static code_t insEncodeReg_Vt(regNumber reg);

// Returns an encoding for the specified register used in the 'Vn' position
static code_t insEncodeReg_Vn(regNumber reg);

// Returns an encoding for the specified register used in the 'Vm' position
static code_t insEncodeReg_Vm(regNumber reg);

// Returns the encoding for the Shift Count bits to be used for MIPS64 encodings
static code_t insEncodeShiftCount(ssize_t imm, emitAttr size);

// Returns the encoding to select the datasize for most MIPS64 instructions
static code_t insEncodeDatasize(emitAttr size);

// Returns the encoding to select the datasize for the general load/store MIPS64 instructions
static code_t insEncodeDatasizeLS(code_t code, emitAttr size);

// Returns the encoding to select the datasize for the vector load/store MIPS64 instructions
static code_t insEncodeDatasizeVLS(code_t code, emitAttr size);

// Returns the encoding to select the datasize for bitfield MIPS64 instructions
static code_t insEncodeDatasizeBF(code_t code, emitAttr size);

// Returns the encoding to select the vectorsize for SIMD MIPS64 instructions
static code_t insEncodeVectorsize(emitAttr size);

// Returns the encoding to shift by 'shift' bits for an MIPS64 vector or scalar instruction
static code_t insEncodeVectorShift(emitAttr size, ssize_t shift);

// Returns the encoding to select the 1/2/4/8 byte elemsize for an MIPS64 vector instruction
static code_t insEncodeElemsize(emitAttr size);

// Returns the encoding to select the 4/8 byte elemsize for an MIPS64 float vector instruction
static code_t insEncodeFloatElemsize(emitAttr size);

// Returns the encoding to select the index for an MIPS64 float vector by elem instruction
static code_t insEncodeFloatIndex(emitAttr elemsize, ssize_t index);

// Returns the encoding to select the 'conversion' operation for a type 'fmt' MIPS64 instruction
//static code_t insEncodeConvertOpt(insFormat fmt, insOpts conversion);

// Returns the encoding to have the Rn register of a ld/st reg be Pre/Post/Not indexed updated
//static code_t insEncodeIndexedOpt(insOpts opt);

// Returns the encoding to have the Rn register of a ld/st pair be Pre/Post/Not indexed updated
//static code_t insEncodePairIndexedOpt(instruction ins, insOpts opt);

// Returns the encoding to apply a Shift Type on the Rm register
//static code_t insEncodeShiftType(insOpts opt);

// Returns the encoding to apply a 12 bit left shift to the immediate
//static code_t insEncodeShiftImm12(insOpts opt);

// Returns the encoding to have the Rm register use an extend operation
//static code_t insEncodeExtend(insOpts opt);

// Returns the encoding to scale the Rm register by {0,1,2,3,4} in an extend operation
static code_t insEncodeExtendScale(ssize_t imm);

// Returns the encoding to have the Rm register be auto scaled by the ld/st size
static code_t insEncodeReg3Scale(bool isScaled);
#endif

// Returns true if 'reg' represents an integer register.
static bool isIntegerRegister(regNumber reg)
{
    return (reg >= REG_INT_FIRST) && (reg <= REG_INT_LAST);
}

// Returns true if 'value' is a legal unsigned immediate 8 bit encoding (such as for fMOV).
static bool isValidUimm8(ssize_t value)
{
#if 0
    return (0 <= value) && (value <= 0xFFLL);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
};

// Returns true if 'value' is a legal unsigned immediate 12 bit encoding (such as for CMP, CMN).
static bool isValidUimm12(ssize_t value)
{
#if 0
    return (0 <= value) && (value <= 0xFFFLL);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
};

// Returns true if 'value' is a legal unsigned immediate 16 bit encoding (such as for MOVZ, MOVN, MOVK).
static bool isValidUimm16(ssize_t value)
{
#if 0
    return (0 <= value) && (value <= 0xFFFFLL);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
};

// Returns true if 'value' is a legal signed immediate 26 bit encoding (such as for B or BL).
static bool isValidSimm26(ssize_t value)
{
#if 0
    return (-0x2000000LL <= value) && (value <= 0x1FFFFFFLL);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
};

// Returns true if 'value' is a legal signed immediate 19 bit encoding (such as for B.cond, CBNZ, CBZ).
static bool isValidSimm19(ssize_t value)
{
#if 0
    return (-0x40000LL <= value) && (value <= 0x3FFFFLL);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
};

// Returns true if 'value' is a legal signed immediate 16 bit encoding.
static bool isValidSimm16(ssize_t value)
{
    return -( ((int)1) << 15 ) <= value && value < ( ((int)1) << 15 );
};

// Returns true if 'value' is a legal signed immediate 14 bit encoding (such as for TBNZ, TBZ).
static bool isValidSimm14(ssize_t value)
{
#if 0
    return (-0x2000LL <= value) && (value <= 0x1FFFLL);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
};

// Returns true if 'value' represents a valid 'bitmask immediate' encoding.
static bool isValidImmNRS(size_t value, emitAttr size)
{
#if 0
    return (value >= 0) && (value < 0x2000);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
} // any unsigned 13-bit immediate

// Returns true if 'value' represents a valid 'halfword immediate' encoding.
static bool isValidImmHWVal(size_t value, emitAttr size)
{
#if 0
    return (value >= 0) && (value < 0x40000);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
} // any unsigned 18-bit immediate

// Returns true if 'value' represents a valid 'byteShifted immediate' encoding.
static bool isValidImmBSVal(size_t value, emitAttr size)
{
#if 0
    return (value >= 0) && (value < 0x800);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
} // any unsigned 11-bit immediate

//  For the given 'ins' returns the reverse instruction, if one exists, otherwise returns INS_INVALID
static instruction insReverse(instruction ins);

//  For the given 'datasize' and 'elemsize' returns the insOpts that specifies the vector register arrangement
//static insOpts optMakeArrangement(emitAttr datasize, emitAttr elemsize);

//    For the given 'datasize' and 'opt' returns true if it specifies a valid vector register arrangement
static bool isValidArrangement(emitAttr datasize, insOpts opt);

//  For the given 'arrangement' returns the 'datasize' specified by the vector register arrangement
static emitAttr optGetDatasize(insOpts arrangement);

//  For the given 'arrangement' returns the 'elemsize' specified by the vector register arrangement
static emitAttr optGetElemsize(insOpts arrangement);

//  For the given 'arrangement' returns the 'widen-arrangement' specified by the vector register arrangement
static insOpts optWidenElemsize(insOpts arrangement);

//  For the given 'conversion' returns the 'dstsize' specified by the conversion option
static emitAttr optGetDstsize(insOpts conversion);

//  For the given 'conversion' returns the 'srcsize' specified by the conversion option
static emitAttr optGetSrcsize(insOpts conversion);

//    For the given 'datasize', 'elemsize' and 'index' returns true, if it specifies a valid 'index'
//    for an element of size 'elemsize' in a vector register of size 'datasize'
static bool isValidVectorIndex(emitAttr datasize, emitAttr elemsize, ssize_t index);

/************************************************************************/
/*           Public inline informational methods                        */
/************************************************************************/

public:
// true if this 'imm' can be encoded as a input operand to a mov instruction
static bool emitIns_valid_imm_for_mov(INT64 imm, emitAttr size);

// true if this 'immDbl' can be encoded as a input operand to MOV.fmt instruction
static bool emitIns_valid_imm_for_fmov(double immDbl);

// true if this 'imm' can be encoded as a input operand to an add instruction
static bool emitIns_valid_imm_for_add(INT64 imm, emitAttr size = EA_8BYTE);

// true if this 'imm' can be encoded as a input operand to a cmp instruction
static bool emitIns_valid_imm_for_cmp(INT64 imm, emitAttr size);

// true if this 'imm' can be encoded as a input operand to an alu instruction
static bool emitIns_valid_imm_for_alu(INT64 imm, emitAttr size);

// true if this 'imm' can be encoded as the offset in a ldr/str instruction
static bool emitIns_valid_imm_for_ldst_offset(INT64 imm, emitAttr size);

// true if 'imm' can use the left shifted by 12 bits encoding
static bool canEncodeWithShiftImmBy12(INT64 imm);

// Normalize the 'imm' so that the upper bits, as defined by 'size' are zero
static INT64 normalizeImm64(INT64 imm, emitAttr size);

// Normalize the 'imm' so that the upper bits, as defined by 'size' are zero
static INT32 normalizeImm32(INT32 imm, emitAttr size);

// true if 'imm' can be encoded using a 'bitmask immediate', also returns the encoding if wbBMI is non-null
static bool canEncodeBitMaskImm(INT64 imm, emitAttr size, emitter::bitMaskImm* wbBMI = nullptr);

// true if 'imm' can be encoded using a 'halfword immediate', also returns the encoding if wbHWI is non-null
static bool canEncodeHalfwordImm(INT64 imm, emitAttr size, emitter::halfwordImm* wbHWI = nullptr);

// true if 'imm' can be encoded using a 'byteShifted immediate', also returns the encoding if wbBSI is non-null
static bool canEncodeByteShiftedImm(INT64 imm, emitAttr size, bool allow_MSL, emitter::byteShiftedImm* wbBSI = nullptr);

// true if 'immDbl' can be encoded using a 'float immediate', also returns the encoding if wbFPI is non-null
static bool canEncodeFloatImm8(double immDbl, emitter::floatImm8* wbFPI = nullptr);

// Returns the number of bits used by the given 'size'.
inline static unsigned getBitWidth(emitAttr size)
{
    assert(size <= EA_8BYTE);
    return (unsigned)size * BITS_PER_BYTE;
}

// Returns true if the imm represents a valid bit shift or bit position for the given 'size' [0..31] or [0..63]
inline static unsigned isValidImmShift(ssize_t imm, emitAttr size)
{
    return (imm >= 0) && (imm < getBitWidth(size));
}

inline static bool isValidGeneralDatasize(emitAttr size)
{
    return (size == EA_8BYTE) || (size == EA_4BYTE);
}

inline static bool isValidScalarDatasize(emitAttr size)
{
#if 0
    return (size == EA_8BYTE) || (size == EA_4BYTE);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool isValidVectorDatasize(emitAttr size)
{
#if 0
    return (size == EA_16BYTE) || (size == EA_8BYTE);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool isValidGeneralLSDatasize(emitAttr size)
{
    return (size == EA_8BYTE) || (size == EA_4BYTE) || (size == EA_2BYTE) || (size == EA_1BYTE);
}

inline static bool isValidVectorLSDatasize(emitAttr size)
{
#if 0
    return (size == EA_16BYTE) || (size == EA_8BYTE) || (size == EA_4BYTE) || (size == EA_2BYTE) || (size == EA_1BYTE);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool isValidVectorLSPDatasize(emitAttr size)
{
#if 0
    return (size == EA_16BYTE) || (size == EA_8BYTE) || (size == EA_4BYTE);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool isValidVectorElemsize(emitAttr size)
{
#if 0
    return (size == EA_8BYTE) || (size == EA_4BYTE) || (size == EA_2BYTE) || (size == EA_1BYTE);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool isValidVectorFcvtsize(emitAttr size)
{
#if 0
    return (size == EA_8BYTE) || (size == EA_4BYTE) || (size == EA_2BYTE);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool isValidVectorElemsizeFloat(emitAttr size)
{
#if 0
    return (size == EA_8BYTE) || (size == EA_4BYTE);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool isGeneralRegister(regNumber reg)
{
    // FIXME: Excludes REG_R0
    //return (reg > REG_INT_FIRST) && (reg <= REG_RA);

    return (reg >= REG_INT_FIRST) && (reg <= REG_RA);
}

inline static bool isGeneralRegisterOrR0(regNumber reg)
{
    return (reg >= REG_FIRST) && (reg <= REG_RA);
} // Includes REG_R0

inline static bool isGeneralRegisterOrSP(regNumber reg)
{
    return isGeneralRegister(reg) || (reg == REG_SP);
} // Includes REG_SP, Excludes REG_R0

inline static bool isVectorRegister(regNumber reg)
{
    //FIXME for mips.
    return false;
}

inline static bool isFloatReg(regNumber reg)
{
    return (reg >= REG_FP_FIRST && reg <= REG_FP_LAST);
}

#if 0
inline static bool insOptsNone(insOpts opt)
{
    return (opt == INS_OPTS_NONE);
}

inline static bool insOptsIndexed(insOpts opt)
{
    return (opt == INS_OPTS_PRE_INDEX) || (opt == INS_OPTS_POST_INDEX);
}

inline static bool insOptsPreIndex(insOpts opt)
{
    return (opt == INS_OPTS_PRE_INDEX);
}

inline static bool insOptsPostIndex(insOpts opt)
{
    return (opt == INS_OPTS_POST_INDEX);
}

inline static bool insOptsLSL12(insOpts opt) // special 12-bit shift only used for imm12
{
#if 0
    return (opt == INS_OPTS_LSL12);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsAnyShift(insOpts opt)
{
#if 0
    return ((opt >= INS_OPTS_LSL) && (opt <= INS_OPTS_ROR));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsAluShift(insOpts opt) // excludes ROR
{
#if 0
    return ((opt >= INS_OPTS_LSL) && (opt <= INS_OPTS_ASR));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsVectorImmShift(insOpts opt)
{
#if 0
    return ((opt == INS_OPTS_LSL) || (opt == INS_OPTS_MSL));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsLSL(insOpts opt)
{
#if 0
    return (opt == INS_OPTS_LSL);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsLSR(insOpts opt)
{
#if 0
    return (opt == INS_OPTS_LSR);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsASR(insOpts opt)
{
#if 0
    return (opt == INS_OPTS_ASR);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsROR(insOpts opt)
{
#if 0
    return (opt == INS_OPTS_ROR);
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsAnyExtend(insOpts opt)
{
#if 0
    return ((opt >= INS_OPTS_UXTB) && (opt <= INS_OPTS_SXTX));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsLSExtend(insOpts opt)
{
#if 0
    return ((opt == INS_OPTS_NONE) || (opt == INS_OPTS_LSL) || (opt == INS_OPTS_UXTW) || (opt == INS_OPTS_SXTW) ||
            (opt == INS_OPTS_UXTX) || (opt == INS_OPTS_SXTX));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOpts32BitExtend(insOpts opt)
{
#if 0
    return ((opt == INS_OPTS_UXTW) || (opt == INS_OPTS_SXTW));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOpts64BitExtend(insOpts opt)
{
#if 0
    return ((opt == INS_OPTS_UXTX) || (opt == INS_OPTS_SXTX));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsAnyArrangement(insOpts opt)
{
#if 0
    return ((opt >= INS_OPTS_8B) && (opt <= INS_OPTS_2D));
#endif
    assert(!"unimplemented on MIPS yet");
    return 0;
}

inline static bool insOptsConvertFloatToFloat(insOpts opt)
{
    return ((opt >= INS_OPTS_S_TO_D) && (opt <= INS_OPTS_D_TO_H));
}

inline static bool insOptsConvertFloatToInt(insOpts opt)
{
    return ((opt >= INS_OPTS_S_TO_4BYTE) && (opt <= INS_OPTS_D_TO_8BYTE));
}

inline static bool insOptsConvertIntToFloat(insOpts opt)
{
    return ((opt >= INS_OPTS_4BYTE_TO_S) && (opt <= INS_OPTS_8BYTE_TO_D));
}
#endif

static bool isValidImmCond(ssize_t imm);
static bool isValidImmCondFlags(ssize_t imm);
static bool isValidImmCondFlagsImm5(ssize_t imm);

/************************************************************************/
/*           The public entry points to output instructions             */
/************************************************************************/

public:
void emitIns(instruction ins);

#if 1
/* FIXME for MIPS: should confirm and implements. */
void emitIns_S_R(instruction ins, emitAttr attr, regNumber ireg, int varx, int offs);
void emitIns_R_S(instruction ins, emitAttr attr, regNumber ireg, int varx, int offs);

void emitIns_I(instruction ins, emitAttr attr, ssize_t imm);
void emitIns_I_I(instruction ins, emitAttr attr, ssize_t imm1, ssize_t imm2);

void emitIns_R_I_I(
    instruction ins, emitAttr attr, regNumber reg1, ssize_t hint, ssize_t off, insOpts opt = INS_OPTS_NONE);

void emitIns_R(instruction ins, emitAttr attr, regNumber reg);

void emitIns_R_I(instruction ins, emitAttr attr, regNumber reg, ssize_t imm, insOpts opt = INS_OPTS_NONE);

void emitIns_R_R(instruction ins, emitAttr attr, regNumber reg1, regNumber reg2, insOpts opt = INS_OPTS_NONE);

void emitIns_R_R(instruction ins, emitAttr attr, regNumber reg1, regNumber reg2, insFlags flags)
{
    emitIns_R_R(ins, attr, reg1, reg2);
}

void emitIns_R_R_I(
    instruction ins, emitAttr attr, regNumber reg1, regNumber reg2, ssize_t imm, insOpts opt = INS_OPTS_NONE);

// Checks for a large immediate that needs a second instruction
void emitIns_R_R_Imm(instruction ins, emitAttr attr, regNumber reg1, regNumber reg2, ssize_t imm);

void emitIns_R_R_R(
    instruction ins, emitAttr attr, regNumber reg1, regNumber reg2, regNumber reg3, insOpts opt = INS_OPTS_NONE);

void emitIns_R_R_R_I(instruction ins,
                     emitAttr    attr,
                     regNumber   reg1,
                     regNumber   reg2,
                     regNumber   reg3,
                     ssize_t     imm,
                     insOpts     opt      = INS_OPTS_NONE,
                     emitAttr    attrReg2 = EA_UNKNOWN);

void emitIns_R_R_R_Ext(instruction ins,
                       emitAttr    attr,
                       regNumber   reg1,
                       regNumber   reg2,
                       regNumber   reg3,
                       insOpts     opt         = INS_OPTS_NONE,
                       int         shiftAmount = -1);

void emitIns_R_R_I_I(instruction ins, emitAttr attr, regNumber reg1, regNumber reg2, int imm1, int imm2);

void emitIns_R_R_R_R(instruction ins, emitAttr attr, regNumber reg1, regNumber reg2, regNumber reg3, regNumber reg4);

//void emitIns_BARR(instruction ins, insBarrier barrier);

void emitIns_C(instruction ins, emitAttr attr, CORINFO_FIELD_HANDLE fdlHnd, int offs);

void emitIns_S(instruction ins, emitAttr attr, int varx, int offs);

void emitIns_S_S_R_R(
    instruction ins, emitAttr attr, emitAttr attr2, regNumber ireg, regNumber ireg2, int varx, int offs);

//void emitIns_R_R_S(
//    instruction ins, emitAttr attr, regNumber ireg, regNumber ireg2, int sa);

void emitIns_R_R_S_S(
    instruction ins, emitAttr attr, emitAttr attr2, regNumber ireg, regNumber ireg2, int varx, int offs);

void emitIns_S_I(instruction ins, emitAttr attr, int varx, int offs, int val);

void emitIns_R_C(
    instruction ins, emitAttr attr, regNumber reg, regNumber tmpReg, CORINFO_FIELD_HANDLE fldHnd, int offs);

void emitIns_R_L(instruction ins, emitAttr attr, BasicBlock* dst, regNumber reg);

void emitIns_J_R(instruction ins, emitAttr attr, BasicBlock* dst, regNumber reg);

void emitIns_C_R(instruction ins, emitAttr attr, CORINFO_FIELD_HANDLE fldHnd, regNumber reg, int offs);

void emitIns_C_I(instruction ins, emitAttr attr, CORINFO_FIELD_HANDLE fdlHnd, ssize_t offs, ssize_t val);

void emitIns_R_D(instruction ins, emitAttr attr, unsigned offs, regNumber reg);

void emitIns_J_R_I(instruction ins, emitAttr attr, BasicBlock* dst, regNumber reg, int instrCount);

void emitIns_I_AR(instruction ins, emitAttr attr, int val, regNumber reg, int offs);

void emitIns_R_AR(instruction ins, emitAttr attr, regNumber ireg, regNumber reg, int offs);

void emitIns_R_AI(instruction ins, emitAttr attr, regNumber ireg, ssize_t disp);

void emitIns_AR_R(instruction ins, emitAttr attr, regNumber ireg, regNumber reg, int offs);

void emitIns_R_ARR(instruction ins, emitAttr attr, regNumber ireg, regNumber reg, regNumber rg2, int disp);

void emitIns_ARR_R(instruction ins, emitAttr attr, regNumber ireg, regNumber reg, regNumber rg2, int disp);

void emitIns_R_ARX(
    instruction ins, emitAttr attr, regNumber ireg, regNumber reg, regNumber rg2, unsigned mul, int disp);
#endif

enum EmitCallType
{

    // I have included here, but commented out, all the values used by the x86 emitter.
    // However, MIPS has a much reduced instruction set, and so the MIPS emitter only
    // supports a subset of the x86 variants.  By leaving them commented out, it becomes
    // a compile time error if code tries to use them (and hopefully see this comment
    // and know why they are unavailible on MIPS), while making it easier to stay
    // in-sync with x86 and possibly add them back in if needed.

    EC_FUNC_TOKEN, //   Direct call to a helper/static/nonvirtual/global method
                   //  EC_FUNC_TOKEN_INDIR,    // Indirect call to a helper/static/nonvirtual/global method
    EC_FUNC_ADDR,  // Direct call to an absolute address

    //  EC_FUNC_VIRTUAL,        // Call to a virtual method (using the vtable)
    EC_INDIR_R, // Indirect call via register
                //  EC_INDIR_SR,            // Indirect call via stack-reference (local var)
                //  EC_INDIR_C,             // Indirect call via static class var
                //  EC_INDIR_ARD,           // Indirect call via an addressing mode

    EC_COUNT
};

void emitIns_Call(EmitCallType          callType,
                  CORINFO_METHOD_HANDLE methHnd,
                  INDEBUG_LDISASM_COMMA(CORINFO_SIG_INFO* sigInfo) // used to report call sites to the EE
                  void*            addr,
                  ssize_t          argSize,
                  emitAttr         retSize
                  MULTIREG_HAS_SECOND_GC_RET_ONLY_ARG(emitAttr secondRetSize),
                  VARSET_VALARG_TP ptrVars,
                  regMaskTP        gcrefRegs,
                  regMaskTP        byrefRegs,
                  IL_OFFSETX       ilOffset      = BAD_IL_OFFSET,
                  regNumber        ireg          = REG_NA,
                  regNumber        xreg          = REG_NA,
                  unsigned         xmul          = 0,
                  ssize_t          disp          = 0,
                  bool             isJump        = false);

//void emitOutputCall(insGroup* ig, BYTE* dst, instrDesc* i);
unsigned emitOutputCall(insGroup* ig, BYTE* dst, instrDesc* id, code_t code);
//BYTE* emitOutputLJ(insGroup* ig, BYTE* dst, instrDesc* i);
//BYTE* emitOutputLoadLabel(BYTE* dst, BYTE* srcAddr, BYTE* dstAddr, instrDescJmp* id);
//BYTE* emitOutputShortBranch(BYTE* dst, instruction ins, insFormat fmt, ssize_t distVal, instrDescJmp* id);
//BYTE* emitOutputShortAddress(BYTE* dst, instruction ins, insFormat fmt, ssize_t distVal, regNumber reg);
//BYTE* emitOutputShortConstant(
//    BYTE* dst, instruction ins, insFormat fmt, ssize_t distVal, regNumber reg, emitAttr opSize);

#endif // _TARGET_MIPS64_