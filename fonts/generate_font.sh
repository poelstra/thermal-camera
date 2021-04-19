#!/bin/sh

# (Re-)generate font of 'normal' characters, plus any needed symbols.
#
# Requires NodeJS to be installed.
# See https://github.com/lvgl/lv_font_conv

set -e

default_symbols="61441,61448,61451,61452,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,61502,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650"
custom_symbols="0xf065,0xf2c9,0xf492,0xf548,0xf1fb"

npx lv_font_conv \
    --no-compress --no-prefilter --bpp 4 --size 16 \
    --font Montserrat-Medium.ttf -r 0x20-0x7F,0xB0,0x2022 \
    --font FontAwesome5-Solid+Brands+Regular.woff -r "$default_symbols,$custom_symbols" \
    --format lvgl --lv-include "lvgl.h" -o ../src/font_montserrat_16.c --force-fast-kern-format
