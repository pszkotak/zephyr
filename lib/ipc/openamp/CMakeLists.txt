#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

zephyr_include_directories_ifdef(CONFIG_IPC_BACKEND_OPENAMP .)
zephyr_sources_ifdef(CONFIG_IPC_BACKEND_OPENAMP multi_instance.c)

zephyr_linker_sources_ifdef(CONFIG_IPC_BACKEND_OPENAMP
  SECTIONS
  multi_instance.ld
)