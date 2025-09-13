# Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
# SPDX-License-Identifier: Apache-2.0

board_runner_args(openocd "--config=${BOARD_DIR}/support/rm57lx.cfg")
board_runner_args(openocd "--use-elf")
board_runner_args(openocd "--verify")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
