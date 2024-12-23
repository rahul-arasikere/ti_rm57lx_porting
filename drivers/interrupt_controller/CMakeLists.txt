# Copyrights 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
# SPDX-License-Identifier: Apache-2.0

zephyr_library_amend()
zephyr_library_sources_ifdef(CONFIG_TI_HERCULES_VIM intc_ti_hercules.c)
