# Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
# SPDX-License-Identifier: Apache-2.0

board_runner_args(openocd "transport select jtag")
board_runner_args(openocd "source [find xds110.cfg")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
