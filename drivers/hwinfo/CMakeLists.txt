# Copyrights 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
# SPDX-License-Identifier: Apache-2.0

zephyr_amend_library()
zephyr_library_sources_ifdef(CONFIG_HWINFO_TI_HERCULES hwinfo_ti_hercules.c)
