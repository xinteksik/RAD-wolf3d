CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic
INCLUDES = -Isrc
all: test
SOURCES = src/radwolf_frame.c src/radwolf_input.c src/radwolf_rad_input.c src/radwolf_platform.c src/radwolf_video.c src/radwolf_file.c src/radwolf_stdio_file.c tests/test_bridge.c
SOURCES += tests/test_file.c
build/test_bridge: $(SOURCES) $(wildcard src/*.h)
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $@
build/test_audio: src/radwolf_audio.c src/radwolf_audio_config.c tests/test_audio.c src/baremetal/SDL_mixer.h
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) src/radwolf_audio.c src/radwolf_audio_config.c tests/test_audio.c -o $@
build/test_audio_config: src/radwolf_audio_config.c tests/test_audio_config.c
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@
test-opl:
	mkdir -p build
	$(CXX) -std=c++11 -O2 -DUSE_GPL -Isrc/baremetal tests/test_opl.cpp src/radwolf_opl.cpp upstream/Wolf4SDL/dosbox/dbopl.cpp -lm -o build/test_opl
	./build/test_opl
test-audio-startup:
	mkdir -p build
	$(CC) -std=gnu99 -O2 -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL -c upstream/Wolf4SDL/id_sd.c -o build/id_sd_host.o
	$(CC) -std=gnu99 -O2 -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL -c tests/test_audio_startup.c -o build/audio_startup_host.o
	$(CC) -std=gnu99 -O2 -Isrc -c src/radwolf_audio.c -o build/audio_host.o
	$(CC) -std=gnu99 -O2 -Isrc -c src/radwolf_audio_config.c -o build/audio_config_host.o
	$(CXX) -std=c++11 -O2 -DUSE_GPL -Isrc/baremetal build/id_sd_host.o build/audio_startup_host.o build/audio_host.o build/audio_config_host.o src/radwolf_opl.cpp upstream/Wolf4SDL/dosbox/dbopl.cpp -lm -o build/test_audio_startup
	./build/test_audio_startup
test: build/test_bridge build/test_audio build/test_audio_config
	./build/test_bridge
	./build/test_audio
	./build/test_audio_config
	python3 -m unittest discover -s tests -p 'test_tools.py' -v
	python3 -m unittest discover -s tests -p 'test_sid_timer.py' -v
	python3 -m unittest discover -s tests -p 'test_sid_stereo.py' -v
clean:
	rm -rf build
.PHONY: all test test-opl test-audio-startup clean
