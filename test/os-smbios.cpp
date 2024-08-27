// -*- C++ -*-

#include "../../base/assert.hpp"
#include "../basics.hpp"
#include "../smbios.hpp"

#include <fstream>
#include <iostream>
#include <string_view>

#define ASSERT DMITIGR_ASSERT

int main()
{
  try {
    namespace fw = dmitigr::os::firmware;
    using std::cout;
    using std::endl;

    const auto smbios = fw::Smbios_table::from_system();
#ifdef _WIN32
    {
      std::ofstream out{"smbios.bin", std::ios::binary};
      out.write(reinterpret_cast<const char*>(smbios.raw().data()),
        smbios.raw().size());
    }
#endif

#ifdef _WIN32
    const auto header = smbios.header();
    cout << "Used 2.0 calling method: " << header.used_20_calling_method << endl;
    cout << "Major version: " << header.major_version << endl;
    cout << "Minor version: " << header.minor_version << endl;
    cout << "DMI revision: " << header.dmi_revision << endl;
    cout << "Length: " << header.length << endl;
#endif

    const auto bios_info = smbios.bios_info();
    cout << "BIOS vendor: " << bios_info.vendor.value_or("") << endl;
    cout << "BIOS version: " << bios_info.version.value_or("") << endl;
    cout << "BIOS release date: " << bios_info.release_date.value_or("") << endl;
    cout << "BIOS ROM size: " << static_cast<int>(bios_info.rom_size) << endl;

    const auto sys_info = smbios.sys_info();
    cout << "Manufacturer: " << sys_info.manufacturer.value_or("") << endl;
    cout << "Product: " << sys_info.product.value_or("") << endl;
    cout << "Version: " << sys_info.version.value_or("") << endl;
    cout << "Serial number: " << sys_info.serial_number.value_or("") << endl;
    cout << "UUID: " << sys_info.uuid.to_string() << endl;

    if (const auto bb_info = smbios.baseboard_info()) {
      cout << "Manufacturer: " << bb_info->manufacturer.value_or("") << endl;
      cout << "Product: " << bb_info->product.value_or("") << endl;
      cout << "Version: " << bb_info->version.value_or("") << endl;
      cout << "Serial number: " << bb_info->serial_number.value_or("") << endl;
    } else
      cout << "Baseboard info is not provided." << endl;

#ifdef _WIN32
    {
      const auto processors = smbios.processors_info();
      for (const auto& info : processors) {
        std::cout << " ------------------------------------------------ " << std::endl;
        std::cout << "socket: " << info.socket.value_or("NULL") << std::endl;
        std::cout << "type: " << (int)info.type << std::endl;
        std::cout << "family: " << (int)info.family << std::endl;
        std::cout << "manufacturer: " << info.manufacturer.value_or("NULL") << std::endl;
        std::cout << "id: " << info.id << std::endl;
        std::cout << "processor_version: " << info.version.value_or("NULL") << std::endl;
        std::cout << "voltage: " << (int)info.voltage << std::endl;
        std::cout << "external_clock: " << info.external_clock << std::endl;
        std::cout << "max_speed: " << info.max_speed << std::endl;
        std::cout << "current_speed: " << info.current_speed << std::endl;
        std::cout << "status: " << (int)info.status << std::endl;
        std::cout << "processor_upgrade: " << (int)info.upgrade << std::endl;
        std::cout << "l1_cache_handle: " << info.l1_cache_handle << std::endl;
        std::cout << "l2_cache_handle: " << info.l2_cache_handle << std::endl;
        std::cout << "l3_cache_handle: " << info.l3_cache_handle << std::endl;
        std::cout << "serial_number: " << info.serial_number.value_or("NULL") << std::endl;
        std::cout << "asset_tag: " << info.asset_tag.value_or("NULL") << std::endl;
        std::cout << "part_number: " << info.part_number.value_or("NULL") << std::endl;
        std::cout << "core_count: " << (int)info.core_count << std::endl;
        std::cout << "core_enabled: " << (int)info.core_enabled << std::endl;
        std::cout << "thread_count: " << (int)info.thread_count << std::endl;
        std::cout << "characteristics: " << info.characteristics << std::endl;
        std::cout << "family_2: " << (std::uint64_t)info.family_2 << std::endl;
        std::cout << "core_count_2: " << info.core_count_2 << std::endl;
        std::cout << "core_enabled_2: " << info.core_enabled_2 << std::endl;
        std::cout << "thread_count_2: " << info.thread_count_2 << std::endl;
        std::cout << "thread_enabled: " << info.thread_enabled << std::endl;
        std::cout << " ------------------------------------------------ " << std::endl;
        std::cout << std::endl;
      }
    }
#endif

  } catch (const std::exception& e) {
    std::clog << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::clog << "unknown error" << std::endl;
    return 2;
  }
}
