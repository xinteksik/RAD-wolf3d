CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror -pedantic
INCLUDES = -Isrc
all: test
SOURCES = src/radwolf_frame.c src/radwolf_input.c src/radwolf_rad_input.c src/radwolf_platform.c src/radwolf_video.c src/radwolf_file.c src/radwolf_stdio_file.c tests/test_bridge.c
SOURCES += tests/test_file.c
SOURCES += src/radwolf_map.c src/radwolf_map_history.c tests/test_map.c tests/test_map_history.c
SOURCES += tests/test_gray.c
SOURCES += src/radwolf_surface.c tests/test_surface.c
SOURCES += src/radwolf_capture.c tests/test_capture.c
SOURCES += src/radwolf_video_config.c tests/test_video_config.c
SOURCES += src/radwolf_flash.c tests/test_flash.c
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
	$(CC) -std=gnu99 -O2 -c src/radwolf_midi.c -o build/midi_host.o
	$(CC) $(CFLAGS) -c src/radwolf_downsample.c -o build/downsample_host.o
	$(CXX) -std=c++11 -O2 -DUSE_GPL -Isrc/baremetal tests/test_opl.cpp src/radwolf_opl.cpp build/midi_host.o build/downsample_host.o upstream/Wolf4SDL/dosbox/dbopl.cpp -lm -o build/test_opl
	./build/test_opl
test-audio-startup:
	mkdir -p build
	$(CC) -std=gnu99 -O2 -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL -c upstream/Wolf4SDL/id_sd.c -o build/id_sd_host.o
	$(CC) -std=gnu99 -O2 -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL -c tests/test_audio_startup.c -o build/audio_startup_host.o
	$(CC) -std=gnu99 -O2 -Isrc -c src/radwolf_audio.c -o build/audio_host.o
	$(CC) -std=gnu99 -O2 -Isrc -c src/radwolf_audio_config.c -o build/audio_config_host.o
	$(CC) -std=gnu99 -O2 -c src/radwolf_midi.c -o build/midi_host.o
	$(CC) $(CFLAGS) -c src/radwolf_downsample.c -o build/downsample_host.o
	$(CXX) -std=c++11 -O2 -DUSE_GPL -Isrc/baremetal build/id_sd_host.o build/audio_startup_host.o build/audio_host.o build/audio_config_host.o build/midi_host.o build/downsample_host.o src/radwolf_opl.cpp upstream/Wolf4SDL/dosbox/dbopl.cpp -lm -o build/test_audio_startup
	./build/test_audio_startup
test-flash-engine:
	mkdir -p build
	$(CC) -std=gnu99 -O2 -Werror=implicit-function-declaration -ffunction-sections -fdata-sections -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL tests/test_flash_engine.c upstream/Wolf4SDL/wl_play.c src/radwolf_flash.c -Wl,--gc-sections -o build/test_flash_engine
	./build/test_flash_engine
build/test_downsample: src/radwolf_downsample.c src/radwolf_downsample_coeffs.h tests/test_downsample.c src/radwolf_downsample.h
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) src/radwolf_downsample.c tests/test_downsample.c -lm -o $@
build/id_us_menu_host.o: upstream/Wolf4SDL/id_us.c
	mkdir -p build
	$(CC) -std=gnu99 -O2 -ffunction-sections -fdata-sections -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL -c $< -o $@
build/test_menu_keyboard: src/radwolf_wolf_event_input.c src/radwolf_input.c tests/test_menu_keyboard.c build/id_us_menu_host.o
	mkdir -p build
	$(CC) -std=gnu99 -O2 -Wall -Wextra -Werror -Wno-unused-parameter -ffunction-sections -fdata-sections -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL $^ -Wl,--gc-sections -o $@
build/wl_main_map_host.o: upstream/Wolf4SDL/wl_main.c
	mkdir -p build
	$(CC) -std=gnu99 -O2 -ffunction-sections -fdata-sections -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL -c $< -o $@
build/wolf_map_host.o: src/radwolf_wolf_map.c
	mkdir -p build
	$(CC) -std=gnu99 -O2 -ffunction-sections -fdata-sections -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL -c $< -o $@
build/test_map_save_engine: tests/test_map_save_engine.c src/radwolf_map.c src/radwolf_map_history.c build/wl_main_map_host.o build/wolf_map_host.o
	mkdir -p build
	$(CC) -std=gnu99 -O2 -Wall -Wextra -Werror -Wno-unused-parameter -DRADWOLF_BAREMETAL -Isrc/baremetal -Isrc -Iupstream/Wolf4SDL $^ -Wl,--gc-sections -o $@
build/test_game_edition: src/radwolf_game_edition.c tests/test_game_edition.c src/radwolf_game_edition.h
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) src/radwolf_game_edition.c tests/test_game_edition.c -o $@
test: build/test_game_edition build/test_map_save_engine build/test_menu_keyboard build/test_bridge build/test_audio build/test_audio_config build/test_downsample
	./build/test_game_edition
	./build/test_map_save_engine
	./build/test_menu_keyboard
	./build/test_bridge
	./build/test_audio
	./build/test_audio_config
	./build/test_downsample
	python3 -m unittest discover -s tests -p 'test_tools.py' -v
	python3 -m unittest discover -s tests -p 'test_sid_timer.py' -v
	python3 -m unittest discover -s tests -p 'test_sid_stereo.py' -v
	python3 -m unittest discover -s tests -p 'test_menu_midi.py' -v
	python3 -m unittest discover -s tests -p 'test_windows_midi.py' -v
	python3 -m unittest discover -s tests -p 'test_video_lab.py' -v
clean:
	rm -rf build
.PHONY: all test test-opl test-audio-startup test-flash-engine clean
