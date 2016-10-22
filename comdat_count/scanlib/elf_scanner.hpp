// Copyright (c) 2016 by SN Systems Ltd., Sony Interactive Entertainment Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef ELF_SCANNER_H
#define ELF_SCANNER_H (1)

#include <cstdint>
#include <functional>
#include <string>

#include <gelf.h>

class elf_intf {
public:
    virtual ~elf_intf () {}
    virtual bool elf_is_le (Elf * const elf) = 0;

    virtual Elf_Data * get_data (Elf_Scn * scn, Elf_Data * data) = 0;
    virtual Elf_Scn * elf_getscn (Elf * elf, std::size_t index) = 0;
    virtual Elf_Scn * elf_nextscn (Elf * elf, Elf_Scn * scn) = 0;
    virtual char const * strptr (Elf * const elf, std::size_t section, std::size_t offset) = 0;

    virtual GElf_Shdr getshdr (Elf_Scn * const section) = 0;
    virtual GElf_Sym getsym (Elf_Data * const data, unsigned section_index) = 0;
};


class libelf_intf final : public elf_intf {
public:
    bool elf_is_le (Elf * const elf) override;
    Elf_Data * get_data (Elf_Scn * scn, Elf_Data * data) override;

    Elf_Scn * elf_getscn (Elf * elf, size_t index) override;
    Elf_Scn * elf_nextscn (Elf * elf, Elf_Scn * scn) override;
    char const * strptr (Elf * const elf, std::size_t section, std::size_t offset) override;

    GElf_Shdr getshdr (Elf_Scn * const section) override;
    GElf_Sym getsym (Elf_Data * const data, unsigned section_index) override;
};



class elf_scanner {
public:
    using callback = std::function<void(std::string const &, std::uint64_t)>;

    explicit elf_scanner (Elf * const elf);
    void scan (callback const & cb); // virtual to allow mocking

private:
    Elf * const elf_;
    bool const is_le_;

    void scan_group_section (Elf_Scn * section, GElf_Shdr const & shdr, callback const & cb);
    std::string group_identifier (GElf_Shdr const & group_shdr);


    struct state {
        explicit state (GElf_Shdr const & shdr_)
                : shdr (shdr_) {}
        decltype (GElf_Shdr::sh_size) n{0};
        unsigned member_count{0};
        std::uint64_t total_size{0};
        GElf_Shdr const shdr;
    };
    void record_member (state * const st, std::uint32_t v);

    static std::uint32_t get_le (std::uint8_t const * v);
    static std::uint32_t get_be (std::uint8_t const * v);
    static bool elf_is_le (Elf * const elf);
};
#endif // ELF_SCANNER_H
// eof elf_scanner.h
