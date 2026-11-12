"""Exercise actual RAD timer wrappers against Circle's lifecycle contract."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class TestSidTimer(unittest.TestCase):
    def test_restart_reinitializes_disconnected_timer(self):
        repo = Path(__file__).resolve().parents[1]
        source = repo / "src/radwolf_kernel.cpp"
        compiler = shutil.which("g++")
        if not source.exists() or not compiler:
            self.skipTest("Requires bootstrapped RAD and a host C++ compiler")
        text = source.read_text()
        start = text.index("static bool sidSamplePlayerStopped")
        end = text.index('extern "C" void radMountFileSystem();', start)
        wrappers = text[start:end]
        fixture = r'''
#include <cassert>
#define SOUND_PLAY_IRQ
struct Timer {
    bool initialized = true, fail = false;
    int initializations = 0, stops = 0, starts = 0;
    bool Initialize() {
        assert(!initialized);
        ++initializations;
        if (fail) return false;
        initialized = true;
        return true;
    }
    void Stop() { assert(initialized); initialized = false; ++stops; }
    void Start(int delay) { assert(initialized && delay > 1); ++starts; }
} timer;
Timer *sidtimer = &timer;
'''
        main = r'''
int main() {
    startSIDSamplePlayer();
    assert(timer.starts == 1 && timer.initializations == 0);
    stopSIDSamplePlayer();
    stopSIDSamplePlayer();
    assert(timer.stops == 1);
    startSIDSamplePlayer();
    assert(timer.starts == 2 && timer.initializations == 1);
    stopSIDSamplePlayer();
    timer.fail = true;
    startSIDSamplePlayer();
    assert(timer.starts == 2 && !timer.initialized);
    timer.fail = false;
    startSIDSamplePlayer();
    assert(timer.starts == 3 && timer.initialized);
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            cpp = path / "timer.cpp"
            binary = path / "timer"
            cpp.write_text(fixture + wrappers + main)
            subprocess.run([compiler, "-std=c++11", str(cpp), "-o", str(binary)],
                           check=True, capture_output=True)
            subprocess.run([str(binary)], check=True, timeout=5)
