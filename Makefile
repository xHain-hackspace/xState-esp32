SHELL := /bin/bash

PIO=pio

generate:
	./generate_voice_cmd.sh

build: generate
	pio run --disable-auto-clean

upload: generate
	pio run --disable-auto-clean -t upload
