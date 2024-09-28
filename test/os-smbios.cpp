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

    const auto header = smbios.header();
    cout << "Used 2.0 calling method: " <<
      static_cast<int>(header.used_20_calling_method) << endl;
    cout << "Major version: " << static_cast<int>(header.major_version) << endl;
    cout << "Minor version: " << static_cast<int>(header.minor_version) << endl;
    cout << "DMI revision: " << static_cast<int>(header.dmi_revision) << endl;
    cout << "Length: " << header.length << endl;

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

    {
      cout << "Processors:" << endl;
      const auto processors = smbios.processors_info();
      for (std::size_t i{}; i < processors.size(); ++i) {
        const auto& proc = processors[i];
        cout << "  Processor "<<i<<":" << endl;
        cout << "    socket: " << proc.socket.value_or("NULL") << endl;
        cout << "    type: " << static_cast<int>(proc.type) << endl;
        cout << "    family: " << static_cast<int>(proc.family) << endl;
        cout << "    manufacturer: " << proc.manufacturer.value_or("NULL") << endl;
        cout << "    id: " << proc.id << endl;
        cout << "    processor_version: " << proc.version.value_or("NULL") << endl;
        cout << "    voltage: " << static_cast<int>(proc.voltage) << endl;
        cout << "    external_clock: " << proc.external_clock << endl;
        cout << "    max_speed: " << proc.max_speed << endl;
        cout << "    current_speed: " << proc.current_speed << endl;
        cout << "    status: " << static_cast<int>(proc.status) << endl;
        cout << "    processor_upgrade: " << static_cast<int>(proc.upgrade) << endl;
        cout << "    l1_cache_handle: " << proc.l1_cache_handle << endl;
        cout << "    l2_cache_handle: " << proc.l2_cache_handle << endl;
        cout << "    l3_cache_handle: " << proc.l3_cache_handle << endl;
        cout << "    serial_number: " << proc.serial_number.value_or("NULL") << endl;
        cout << "    asset_tag: " << proc.asset_tag.value_or("NULL") << endl;
        cout << "    part_number: " << proc.part_number.value_or("NULL") << endl;
        cout << "    core_count: " << static_cast<int>(proc.core_count) << endl;
        cout << "    core_enabled: " << static_cast<int>(proc.core_enabled) << endl;
        cout << "    thread_count: " << static_cast<int>(proc.thread_count) << endl;
        cout << "    characteristics: " << proc.characteristics << endl;
        cout << "    family_2: " << static_cast<std::uint64_t>(proc.family_2) << endl;
        cout << "    core_count_2: " << proc.core_count_2 << endl;
        cout << "    core_enabled_2: " << proc.core_enabled_2 << endl;
        cout << "    thread_count_2: " << proc.thread_count_2 << endl;
        cout << "    thread_enabled: " << proc.thread_enabled << endl;
      }
    }
  } catch (const std::exception& e) {
    std::clog << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::clog << "unknown error" << std::endl;
    return 2;
  }
}
