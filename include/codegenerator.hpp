/*
codegenerator.hpp

Copyright (c) 10 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <cstdint>
#include <cassert>

#include <vector>
#include <variant>

#include <typeinfo>

#include "registers.hpp"
#include "addressing.hpp"

namespace x86gen
{

static constexpr uint8_t operand_size_override = 0x66;
static constexpr uint8_t address_size_override = 0x66;

enum JumpCond : uint8_t
{
    JA   = 0x7,
    JAE  = 0x3,
    JB   = 0x2,
    JBE  = 0x6,
    JC   = 0x2,
    JE   = 0x4,
    JG   = 0xF,
    JGE  = 0xD,
    JL   = 0xC,
    JLE  = 0xE,
    JNA  = 0x6,
    JNAE = 0x2,
    JNB  = 0x3,
    JNBE = 0x7,
    JNC  = 0x3,
    JNE  = 0x5,
    JNG  = 0xE,
    JNGE = 0xC,
    JNL  = 0xD,
    JNLE = 0xF,
    JNO  = 0x1,
    JNP  = 0xB,
    JNS  = 0x9,
    JNZ  = 0x5,
    JO   = 0x0,
    JP   = 0xA,
    JPE  = 0xA,
    JPO  = 0xB,
    JS   = 0x8,
    JZ   = 0x4
};

typedef std::variant<Imm8,    // 0
                     Imm16,   // 1
                     Imm32,   // 2
                     Rel8,    // 3
                     Rel16,   // 4
                     Rel32,   // 5
                     GPR8,    // 6
                     GPR16,   // 7
                     GPR32,   // 8
                     ModRM,   // 9
                     JumpCond // A
                     > Operand;

// TODO : find a less "hacky" way to do this
class CodeGenerator;
typedef void(CodeGenerator::*general_gen_func_ptr)(std::vector<Operand>);

// This struct is used in the 'operator<<' interface, to set it's operation
struct Opcode {
       bool operator==(Opcode &other) {return m_argument_number == other.m_argument_number &&
                                       m_parsing_complete_callback == other.m_parsing_complete_callback;};
       unsigned int m_argument_number = 0;
       general_gen_func_ptr m_parsing_complete_callback;
};

class CodeGenerator
{
public:
    CodeGenerator();

    const std::vector<uint8_t>& data() const&
    { return m_data; }
    std::vector<uint8_t>& data() &
    { return m_data; }

    std::vector<uint8_t>&& data() &&
    { return std::move(m_data); }

    size_t current_index() const { return m_idx; }

public:
       /* Those general overloads decide by themselves what overload to execute */
       void general_std(std::vector<Operand>);
       void general_cld(std::vector<Operand>);
       void general_repe_scasb(std::vector<Operand>);
       void general_repe_scasw(std::vector<Operand>);
       void general_repe_scasd(std::vector<Operand>);
       void general_repne_scasb(std::vector<Operand>);
       void general_repne_scasw(std::vector<Operand>);
       void general_repne_scasd(std::vector<Operand>);
       void general_call(std::vector<Operand>);
       void general_ret(std::vector<Operand>);
       void general_far_ret(std::vector<Operand>);
       void general_cmp(std::vector<Operand>);
       void general_sub(std::vector<Operand>);
       void general_add(std::vector<Operand>);
       void general_xor(std::vector<Operand>);
       void general_imul2(std::vector<Operand>);
       void general_imul3(std::vector<Operand>);
       void general_jmp(std::vector<Operand>);
       void general_jcc(std::vector<Operand>);
       void general_mov(std::vector<Operand>);
       void general_lea(std::vector<Operand>);
       void general_push(std::vector<Operand>);
       void general_pop(std::vector<Operand>);
       void general_inc(std::vector<Operand>);
       void general_dec(std::vector<Operand>);
       void general_int3(std::vector<Operand>);
       void general_int(std::vector<Operand>);
       void general_ud2(std::vector<Operand>);

public:
       friend CodeGenerator& operator<<(CodeGenerator& to_apply, Opcode a);
       friend CodeGenerator& operator<<(CodeGenerator& to_apply, Operand a);

private:
       std::vector<Operand> m_operands_consumed;
       Opcode m_current_opcode = {0, &CodeGenerator::general_ud2};

       // Every instruction whose number of operands isn't fixed is aliased here for the sake of intercompatibility
       void imul2(GPR16 a, ModRM b) {imul(a, b);};
       void imul2(GPR32 a, ModRM b) {imul(a, b);};
       void imul3(GPR16 a, ModRM b, Imm8 c) {imul(a, b, c);};
       void imul3(GPR32 a, ModRM b, Imm8 c) {imul(a, b, c);};
       void imul2(GPR16 a, Imm8 b) {imul(a, b);};
       void imul2(GPR32 a, Imm8 b) {imul(a, b);};

public:
    void std();
    void cld();

    void repe_scasb();
    void repe_scasw();
    void repe_scasd();
    void repne_scasb();
    void repne_scasw();
    void repne_scasd();

    void call(Rel16);
    void call(Rel32);
    void call(ModRM);

    void ret();
    void far_ret();

    void cmp(ModRM, Imm8);
    void cmp(ModRM, Imm16);
    void cmp(ModRM, Imm32);
    void cmp(ModRM, GPR8);
    void cmp(ModRM, GPR16);
    void cmp(ModRM, GPR32);
    void cmp(GPR8 , ModRM);
    void cmp(GPR16, ModRM);
    void cmp(GPR32, ModRM);

    void sub(ModRM, Imm8);
    void sub(ModRM, Imm16);
    void sub(ModRM, Imm32);
    void sub(ModRM, GPR8);
    void sub(ModRM, GPR16);
    void sub(ModRM, GPR32);
    void sub(GPR8 , ModRM);
    void sub(GPR16, ModRM);
    void sub(GPR32, ModRM);

    void add(ModRM, Imm8);
    void add(ModRM, Imm16);
    void add(ModRM, Imm32);
    void add(ModRM, GPR8);
    void add(ModRM, GPR16);
    void add(ModRM, GPR32);
    void add(GPR8 , ModRM);
    void add(GPR16, ModRM);
    void add(GPR32, ModRM);

    void xor_(ModRM, Imm8);
    void xor_(ModRM, Imm16);
    void xor_(ModRM, Imm32);
    void xor_(ModRM, GPR8);
    void xor_(ModRM, GPR16);
    void xor_(ModRM, GPR32);
    void xor_(GPR8 , ModRM);
    void xor_(GPR16, ModRM);
    void xor_(GPR32, ModRM);

    void imul(GPR16, ModRM);
    void imul(GPR32, ModRM);
    void imul(GPR16, ModRM, Imm8);
    void imul(GPR32, ModRM, Imm8);
    void imul(GPR16, Imm8);
    void imul(GPR32, Imm8);

    void jmp(Rel8);
    void jmp(Rel16);
    void jmp(Rel32);
    void jmp(ModRM);

    void jcc(JumpCond, Rel8);
    void jcc(JumpCond, Rel16);
    void jcc(JumpCond, Rel32);

    void mov(ModRM, GPR8 );
    void mov(ModRM, GPR16);
    void mov(ModRM, GPR32);
    void mov(GPR8 , ModRM);
    void mov(GPR16, ModRM);
    void mov(GPR32, ModRM);
    void mov(GPR8 , GPR8 );
    void mov(GPR16, GPR16);
    void mov(GPR32, GPR32);
    void mov(GPR8 , Imm8 );
    void mov(GPR16, Imm16);
    void mov(GPR32, Imm32);
    void mov(ModRM, Imm8 );
    void mov(ModRM, Imm16);
    void mov(ModRM, Imm32);

    void lea(GPR16, ModRM);
    void lea(GPR32, ModRM);

    void push(ModRM);
    void push(GPR16);
    void push(GPR32);
    void push(Imm8);
    void push(Imm16);
    void push(Imm32);

    void pop(ModRM);
    void pop(GPR16);
    void pop(GPR32);

    void inc(ModRM);
    void inc(GPR16);
    void inc(GPR32);

    void dec(ModRM);
    void dec(GPR16);
    void dec(GPR32);

    void int3();
    void int_(Imm8);

    void ud2() { emit<uint16_t>(0x0B0F); }

    enum OutputEndianess
    {
        BigEndian,
        LittleEndian
    };

    template <typename T, size_t byte_size = sizeof(T), OutputEndianess endian = OutputEndianess::LittleEndian>
    void emit(typename std::common_type<T>::type value) // by using std::common_type<> we disable argument deduction to force the size to be specified
    {
        static_assert(byte_size <= sizeof(T));

        if (m_idx + byte_size >= m_data.size()) resize_buffer(m_idx + byte_size);
        if constexpr (endian == OutputEndianess::LittleEndian)
        {
            for (size_t i { 0 }; i < byte_size; ++i)
            {
                m_data[m_idx++] = value&0xFF;
                if constexpr (byte_size > 1) value >>= 8;
            }
        }
        else
        {
            for (size_t i { 0 }; i < byte_size; ++i)
            {
                m_data[m_idx + (byte_size-i-1)] = value&0xFF;
                if constexpr (byte_size > 1) value >>= 8;
            }
            m_idx += byte_size;
        }
    }

private:
    void emit_modrm(ModRM rm);

private:
    void resize_buffer(size_t);

    void relocate(size_t new_idx)
    {
        assert(new_idx >= m_idx);
        m_idx = new_idx;
    }

    std::vector<uint8_t> m_data;
    size_t m_idx { 0 };
};
};

#endif // CODEGENERATOR_HPP
