FLAGS += \
        -DTEST \
        -I./eurorack \
        -Wno-unused-local-typedefs

# SOURCES += $(wildcard src/*.cpp)
SOURCES += src/Catatumbo.cpp

SOURCES += src/Puya.cpp

SOURCES += eurorack/stmlib/utils/random.cc
SOURCES += eurorack/stmlib/dsp/atan.cc
SOURCES += eurorack/stmlib/dsp/units.cc
SOURCES += eurorack/tides/generator.cc
SOURCES += eurorack/tides/resources.cc

SOURCES += eurorack/braids/macro_oscillator.cc
SOURCES += eurorack/braids/analog_oscillator.cc
SOURCES += eurorack/braids/digital_oscillator.cc
SOURCES += eurorack/braids/resources.cc
SOURCES += eurorack/braids/quantizer.cc

SOURCES += eurorack/rings/dsp/fm_voice.cc
SOURCES += eurorack/rings/dsp/string_synth_part.cc
SOURCES += eurorack/rings/dsp/string.cc
SOURCES += eurorack/rings/dsp/resonator.cc
SOURCES += eurorack/rings/resources.cc
SOURCES += eurorack/rings/dsp/part.cc

SOURCES += eurorack/clouds/dsp/correlator.cc
SOURCES += eurorack/clouds/dsp/granular_processor.cc
SOURCES += eurorack/clouds/dsp/mu_law.cc
SOURCES += eurorack/clouds/dsp/pvoc/frame_transformation.cc
SOURCES += eurorack/clouds/dsp/pvoc/phase_vocoder.cc
SOURCES += eurorack/clouds/dsp/pvoc/stft.cc
SOURCES += eurorack/clouds/resources.cc 

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets*)

RACK_DIR ?= $()
include $(RACK_DIR)/plugin.mk
