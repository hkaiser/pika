# Copyright (c) 2019 The STE||AR-Group
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(__boost_filesystem)
if(PIKA_FILESYSTEM_WITH_BOOST_FILESYSTEM_COMPATIBILITY)
  set(__boost_filesystem Boost::filesystem)
endif()

set(filesystem_headers pika/modules/filesystem.hpp)

include(pika_add_module)
pika_add_module(
  pika filesystem
  GLOBAL_HEADER_GEN OFF
  HEADERS ${filesystem_headers}
  MODULE_DEPENDENCIES pika_config
  DEPENDENCIES ${__boost_filesystem}
  CMAKE_SUBDIRS examples tests
)
