// Copyright (c) 2016 by Sony Computer Entertainment Inc.
// This file is subject to the terms and conditions defined in file
// 'LICENSE.txt', which is part of this source code package.

#include "worker_error.hpp"

worker_error::worker_error ()
    : std::exception () {
}

char const * worker_error::what() const noexcept {
    return "worker error";
}
// eof worker_error.cpp
