// Copyright (c) 2016 by Sony Interactive Entertainment, Inc.
// This file is subject to the terms and conditions defined in file
// 'LICENSE.txt', which is part of this source code package.

#include "print.hpp"
namespace details {
    ios_printer cout{std::cout};
    ios_printer cerr{std::cerr};
}
// eof print.cpp
