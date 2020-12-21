# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hci_rpmsg)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_NRF_802154_SER_RADIO app PRIVATE src/nrf_802154_serialization_error.c)
