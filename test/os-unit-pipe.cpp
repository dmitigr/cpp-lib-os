// -*- C++ -*-
//
// Copyright 2023 Dmitry Igrishin

#include "../../base/assert.hpp"
#include "../../os/ipc_pipe.hpp"

#include <iostream>

int main()
{
  namespace pp = dmitigr::os::ipc::pp;

  try {
    const auto r = pp::exec_and_wait("head", {"head", "-n", "2"}, "Hello\nWorld\n",
      std::cout, std::cerr);
    DMITIGR_ASSERT(!r);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
