#!/bin/bash

render_wav() {
  say "${1}" -r 210 --data-format=UI8@16000 --channels=1 -o "${2}"
}

output_header() {
  (
    echo -e "#pragma once\n#include <Arduino.h>\n\nunsigned char PROGMEM ${1}[] = {"
    hexdump -v -e '1/1 "0x%02X, " "\n"' "${2}"
    echo "};"
  ) > ${3}
}

TMP_DIR=$(mktemp -d)

render_wav "x-hein is open" "${TMP_DIR}/open.wav"
render_wav "x-hein is open for members" "${TMP_DIR}/members.wav"
render_wav "x-hein is closed" "${TMP_DIR}/closed.wav"

output_header "voice_open" "${TMP_DIR}/open.wav" "./include/voice_open.h"
output_header "voice_members" "${TMP_DIR}/members.wav" "./include/voice_members.h"
output_header "voice_closed" "${TMP_DIR}/closed.wav" "./include/voice_closed.h"

rm -r "${TMP_DIR}"