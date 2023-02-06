#!/bin/bash

SAY=say
FILE_PREFIX=gen
OUTPUT_DIR=./include

render_wav() {
  say "${1}" -r 210 --data-format=I16@44100 --channels=1 -o "${2}"
}

output_header() {
  (
    echo -e "#pragma once\n\nunsigned char PROGMEM ${1}[] = {"
    hexdump -v -e '1/1 "0x%02X, " "\n"' "${2}"
    echo "};"
  ) > ${3}
}

# say cmd is only available in macos
if [ "$(uname -s)" != "Darwin" ]; then
  echo "This script needs to be run under macOS!"
  exit 1
fi
  
TMP_DIR=.

render_wav "x-hein is open" "${TMP_DIR}/open.wav"
render_wav "x-hein is open for members" "${TMP_DIR}/members.wav"
render_wav "x-hein is closed" "${TMP_DIR}/closed.wav"

output_header "voice_open" "${TMP_DIR}/open.wav" "${OUTPUT_DIR}/gen_voice_open.h"
output_header "voice_members" "${TMP_DIR}/members.wav" "${OUTPUT_DIR}/gen_voice_members.h"
output_header "voice_closed" "${TMP_DIR}/closed.wav" "${OUTPUT_DIR}/gen_voice_closed.h"

# render meta header
echo -e '#pragma once\n\n#include "gen_voice_open.h"\n#include "gen_voice_members.h"\n#include "gen_voice_closed.h"\n' > "${OUTPUT_DIR}/gen_voice_data.h"

# rm -r "${TMP_DIR}"
