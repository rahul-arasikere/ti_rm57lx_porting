# Copyrights 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
# SPDX-License-Identifier: Apache-2.0

zephyr_amend_library()
zephyr_library_sources_ifdef(CONFIG_INTERRUPT_CONTROLLER_TI_HERCULES intc_ti_hercules.c)
