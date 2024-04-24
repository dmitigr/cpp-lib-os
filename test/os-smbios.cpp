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
  } catch (const std::exception& e) {
    std::clog << "error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::clog << "unknown error" << std::endl;
    return 2;
  }
}
