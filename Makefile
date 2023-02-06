SHELL := /bin/bash

PIO=pio

generate:
	./generate_voice_cmd.sh

build:
	pio run

upload: generate
	pio run --disable-auto-clean -t upload
