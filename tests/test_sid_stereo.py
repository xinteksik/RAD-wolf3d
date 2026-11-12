"""Check actual second-SID initialization using simulated register accesses."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


class TestSidStereo(unittest.TestCase):
    def test_irq_resynchronizes_before_each_sid_write(self):
        source = Path(__file__).resolve().parents[1] / (
            "src/radwolf_kernel.cpp")
        compiler = shutil.which("g++")
        if not source.exists() or not compiler:
            self.skipTest("Requires bootstrapped RAD and host C++ compiler")
        text = source.read_text()
        start = text.index("void sidSamplePlayIRQ(")
        end = text.index("// Circle CUserTimer::Stop", start)
        implementation = text[start:end].rsplit("#endif", 1)[0]
        poke = next(line for line in text.splitlines()
                    if line.startswith("#define POKE("))
        fixture = r'''
#include <cassert>
#include <cstdint>
using u32 = uint32_t;
#define RADWOLF_BAREMETAL
#define SAMPLE_RATE 22050
#define SOUND_RINGBUF_SIZE 16384
#define DAC_ADDRESS 0xd418
#define USER_CLOCKHZ 1000000
uint64_t soundFirstuSecTick = 1000000;
uint64_t GetuSec() { return 1000000; }
unsigned char soundRingBuffer[16384] = {11};
unsigned char radwolf_right_ring[16384] = {22};
unsigned radwolf_sid2_address = 0xd518;
unsigned radwolf_sid1_address = 0xd418;
static bool prepared;
static unsigned writes, addresses[4], values[4];
static unsigned primary_sample, secondary_sample;
#define WAIT_FOR_CPU_HALFCYCLE { g2 = 0; }
#define WAIT_FOR_VIC_HALFCYCLE { g2 = 0; }
#define RESTART_CYCLE_COUNTER { prepared = true; }
void emuWriteByteREU_p1(u32 &, unsigned address, unsigned value) {
    assert(prepared); prepared = false;
    addresses[writes] = address; values[writes++] = value;
    // Model a broad primary chip select also accepting the secondary write.
    if (address == 0xd518) primary_sample = secondary_sample = value;
    if (address == 0xd418) primary_sample = value;
}
void emuWriteByteREU_p2(u32 &, bool) { assert(!prepared); }
struct CUserTimer { void Start(unsigned delay) { assert(delay > 1); } };
'''
        main = r'''
int main() {
    CUserTimer timer;
    sidSamplePlayIRQ(&timer, nullptr);
    assert(writes == 2);
    assert(addresses[0] == 0xd518 && values[0] == 22);
    assert(addresses[1] == 0xd418 && values[1] == 11);
    assert(primary_sample == 11 && secondary_sample == 22);
    radwolf_sid2_address = 0;
    radwolf_sid1_address = 0xde18;
    sidSamplePlayIRQ(&timer, nullptr);
    assert(writes == 3 && addresses[2] == 0xde18);
}
'''
        with tempfile.TemporaryDirectory() as directory:
            cpp = Path(directory) / "irq.cpp"
            binary = Path(directory) / "irq"
            cpp.write_text(fixture + poke + "\n" + implementation + main)
            subprocess.run([compiler, "-std=c++11", str(cpp), "-o", str(binary)],
                           check=True, capture_output=True)
            subprocess.run([str(binary)], check=True, timeout=5)

    def test_detection_configuration_and_fallback(self):
        source = Path(__file__).resolve().parents[1] / (
            "upstream/RAD-Doom/Source/rad_doom_hijack.cpp")
        compiler = shutil.which("g++")
        if not source.exists() or not compiler:
            self.skipTest("Requires bootstrapped RAD and host C++ compiler")
        text = source.read_text()
        start = text.index("unsigned radwolf_sid2_address = 0;")
        implementation = text[start:text.index("#endif", start)]
        fixture = r'''
#include <cassert>
#include <map>
using u8 = unsigned char;
static std::map<unsigned, unsigned> registers;
static unsigned response = 2, reads;
static const unsigned char lookup6581[256] = {};
static const unsigned char lookup8580[256] = {};
static const unsigned char *mahoneyLUT = lookup8580;
static unsigned char SIDType = (8580 & 255), supportDAC;
#include "radwolf_audio_config.h"
#include "radwolf_ultisid_lut.h"
#define BUS_RESYNC
#define SPOKE(a,v) registers[(a)] = (v)
#define POKE(a,v) SPOKE(a,v)
#define PEEK(a,v) { assert((a) == 0xd51b); (v) = response; ++reads; }
void NOP(unsigned) {}
'''
        main = r'''
int main() {
    assert(radwolf_sid2_initialize(0xd500, 0) == 1);
    assert(reads == 3 && radwolf_sid2_address == 0xd518);
    assert(radwolf_sid2_lut == lookup8580);
    assert(registers[0xd505] == 0x0f && registers[0xd517] == 3);
    response = 3;
    assert(radwolf_sid2_initialize(0xd500, 0) == 1);
    assert(radwolf_sid2_lut == lookup6581);
    response = 0;
    assert(radwolf_sid2_initialize(0xd500, 0) == 0);
    assert(!radwolf_sid2_address && !radwolf_sid2_lut);
    registers.clear();
    assert(!radwolf_sid2_initialize(0xd400, 8580));
    assert(registers.empty());
    assert(radwolf_sid2_initialize(0xd500, 4));
    assert(radwolf_sid2_lut[255] == 15 && registers[0xd517] == 0xf7);
    registers.clear();
    // Model a boot-established oscillator/envelope register state.
    registers[0xd40e] = 0xff;
    registers[0xd40f] = 0xff;
    assert(radwolf_sid1_boot_model() == 8580);
    assert(radwolf_sid1_initialize(0xd400, 8580));
    assert(registers.size() == 2 && registers[0xd40e] == 0xff && registers[0xd40f] == 0xff);
    assert(radwolf_sid1_address == 0xd418 && mahoneyLUT == lookup8580);
    assert(radwolf_sid1_model == 8580);
    assert(radwolf_sid2_initialize(0xd500, 6581));
    assert(radwolf_sid2_lut == lookup6581 && mahoneyLUT == lookup8580);
    assert(radwolf_sid1_initialize(0xd400, RADWOLF_SID_ULTISID));
    assert(mahoneyLUT == radwolf_ultisid_lut && radwolf_sid1_model == 64);
    assert(radwolf_sid2_initialize(0xd500, RADWOLF_SID_ULTISID));
    assert(radwolf_sid2_lut == radwolf_ultisid_lut && radwolf_sid2_model == 64);
    assert(registers[0xd405] == 0x0f && registers[0xd415] == 0xff);
    assert(registers[0xd517] == 3 && registers[0xd504] == 0x49);
}
'''
        with tempfile.TemporaryDirectory() as directory:
            cpp = Path(directory) / "stereo.cpp"
            binary = Path(directory) / "stereo"
            cpp.write_text(fixture + implementation + main)
            subprocess.run([compiler, "-std=c++11", "-I", str(source.parents[3] / "src"), str(cpp), "-o", str(binary)],
                           check=True, capture_output=True)
            subprocess.run([str(binary)], check=True, timeout=5)
