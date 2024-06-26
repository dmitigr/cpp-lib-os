// -*- C++ -*-
//
// Copyright 2024 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_OS_SMBIOS_HPP
#define DMITIGR_OS_SMBIOS_HPP

#include "../base/assert.hpp"
#include "../base/stream.hpp"
#include "../base/traits.hpp"
#include "../rnd/uuid.hpp"
#include "error.hpp"
#ifdef _WIN32
#include "../winbase/exceptions.hpp"
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#ifdef __linux__
#include <fstream>
#endif

namespace dmitigr::os::firmware {

class Smbios_table final {
public:
  using Byte = std::uint8_t;
  using Word = std::uint16_t;
  using Dword = std::uint32_t;
  using Qword = std::uint64_t;

#ifdef _WIN32
  struct Header final {
    Byte used_20_calling_method{};
    Byte major_version{};
    Byte minor_version{};
    Byte dmi_revision{};
    Dword length{};
  };
  static_assert(std::is_standard_layout_v<Header>);
#endif

  struct Structure {
    Byte type{};
    Byte length{};
    Word handle{};
  };
  static_assert(std::is_standard_layout_v<Structure>);

  struct Bios_info final : Structure {
    std::optional<std::string> vendor;
    std::optional<std::string> version;
    std::optional<std::string> release_date;
    Byte rom_size{};
  };

  struct Sys_info final : Structure {
    std::optional<std::string> manufacturer;
    std::optional<std::string> product;
    std::optional<std::string> version;
    std::optional<std::string> serial_number;
    rnd::Uuid uuid{};
  };

  struct Baseboard_info final : Structure {
    std::optional<std::string> manufacturer;
    std::optional<std::string> product;
    std::optional<std::string> version;
    std::optional<std::string> serial_number;
  };

  Smbios_table(const Byte* const data, const std::size_t size)
    : data_(size)
  {
    std::memcpy(data_.data(), data, size);
#ifdef _WIN32
    if (size != header().length)
      throw std::invalid_argument{"invalid SMBIOS firmware table provided"};
#endif
  }

  static Smbios_table from_system()
  {
    Smbios_table result;
    auto& rd = result.data_;
#ifdef _WIN32
    rd.resize(GetSystemFirmwareTable('RSMB', 0, nullptr, 0));
    if (rd.empty() || !GetSystemFirmwareTable('RSMB', 0, rd.data(), rd.size()))
      throw winbase::Sys_exception{"cannot get SMBIOS firmware table"};
#elif __linux__
    const std::filesystem::path dmi_path{"/sys/firmware/dmi/tables/DMI"};
    std::ifstream dmi{dmi_path, std::ios::binary};
    if (!dmi)
      throw std::runtime_error{"cannot open "+dmi_path.string()};

    rd.resize(seekg_size(dmi));
    if (!dmi.read(reinterpret_cast<char*>(rd.data()), rd.size()))
      throw std::runtime_error{"cannot read "+dmi_path.string()};
#else
    #error Unsupported OS family
#endif
    return result;
  }

#ifdef _WIN32
  Header header() const
  {
    return *reinterpret_cast<const Header*>(data_.data());
  }
#endif

  const std::vector<Byte>& raw() const noexcept
  {
    return data_;
  }

  Bios_info bios_info() const
  {
    const auto* const s = structure(0);
    auto result = make_structure<Bios_info>(*s);
    result.vendor = field<std::optional<std::string>>(s, 0x4);
    result.version = field<std::optional<std::string>>(s, 0x5);
    result.release_date = field<std::optional<std::string>>(s, 0x8);
    result.rom_size = field<Byte>(s, 0x9);
    return result;
  }

  Sys_info sys_info() const
  {
    const auto* const s = structure(1);
    auto result = make_structure<Sys_info>(*s);
    result.manufacturer = field<std::optional<std::string>>(s, 0x4);
    result.product = field<std::optional<std::string>>(s, 0x5);
    result.version = field<std::optional<std::string>>(s, 0x6);
    result.serial_number = field<std::optional<std::string>>(s, 0x7);
    result.uuid = field<std::array<Byte, 16>>(s, 0x8);
    return result;
  }

  std::optional<Baseboard_info> baseboard_info() const
  {
    const auto* const s = structure(2, true);
    if (!s)
      return std::nullopt;
    auto result = make_structure<Baseboard_info>(*s);
    result.manufacturer = field<std::optional<std::string>>(s, 0x4);
    result.product = field<std::optional<std::string>>(s, 0x5);
    result.version = field<std::optional<std::string>>(s, 0x6);
    result.serial_number = field<std::optional<std::string>>(s, 0x7);
    return result;
  }

private:
  std::vector<Byte> data_;

  Smbios_table() = default;

  template<class S>
  static S make_structure(const Structure& s)
  {
    static_assert(std::is_base_of_v<Structure, S>);
    S result;
    result.type = s.type;
    result.length = s.length;
    result.handle = s.handle;
    return result;
  }

  const Structure* structure(const Byte type,
    const bool no_throw_if_not_found = false) const
  {
    for (auto* s = first_structure(); s; s = next_structure(s)) {
      if (s->type == type)
        return s;
    }
    if (no_throw_if_not_found)
      return nullptr;
    else
      throw std::runtime_error{"no BIOS information structure of type "
        +std::to_string(type)+" found in SMBIOS"};
  }

  static const char* unformed_section(const Structure* const s) noexcept
  {
    DMITIGR_ASSERT(s);
    return reinterpret_cast<const char*>(s) + s->length;
  }

  const Structure* first_structure() const noexcept
  {
#ifdef _WIN32
    return reinterpret_cast<const Structure*>(data_.data() + sizeof(Header));
#else
    return reinterpret_cast<const Structure*>(data_.data());
#endif
  }

  const Structure* next_structure(const Structure* const s) const noexcept
  {
    DMITIGR_ASSERT(s);
    bool is_prev_char_zero{};
#ifdef _WIN32
    const std::ptrdiff_t length = data_.size() - sizeof(Header);
#else
    const std::ptrdiff_t length = data_.size();
#endif
    const auto* const fst = first_structure();
    for (const char* ptr{unformed_section(s)};
         ptr + 1 - reinterpret_cast<const char*>(fst) < length; ++ptr) {
      if (*ptr == 0) {
        if (is_prev_char_zero)
          return reinterpret_cast<const Structure*>(ptr + 1);
        else
          is_prev_char_zero = true;
      } else
        is_prev_char_zero = false;
    }
    return nullptr;
  }

  template<typename T>
  static T field(const Structure* const s, const std::ptrdiff_t offset)
  {
    DMITIGR_ASSERT(offset >= 0x0);
    using Dt = std::decay_t<T>;
    const Byte* const ptr = reinterpret_cast<const Byte*>(s) + offset;
    if constexpr (std::is_same_v<Dt, std::optional<std::string>>) {
      const int idx = *ptr;
      if (!idx)
        return std::nullopt;
      const char* str = unformed_section(s);
      for (int i{1}; i < idx; ++i) {
        std::string_view view{str};
        str = view.data() + view.size() + 1;
        DMITIGR_ASSERT(*str != 0);
      }
      return std::string{str};
    } else if constexpr (Is_std_array<Dt>::value) {
      using V = typename Dt::value_type;
      if constexpr (std::is_same_v<V, Byte>) {
        Dt result;
        std::memcpy(result.data(), ptr, result.size());
        return result;
      } else
        static_assert(false_value<V>, "unsupported type");
    } else if constexpr (
      std::is_same_v<Dt, Byte>  || std::is_same_v<Dt, Word> ||
      std::is_same_v<Dt, Dword> || std::is_same_v<Dt, Qword>) {
      return *reinterpret_cast<const Dt*>(ptr);
    } else
      static_assert(false_value<T>, "unsupported type");
  }
};

} // namespace dmitigr::os::firmware

#endif  // DMITIGR_OS_SMBIOS_HPP
