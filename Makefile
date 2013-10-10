PROGRAM = planetstrike

# Build type
#BUILDTYPE = debug
BUILDTYPE = release

# Compiler flags
CXXFLAGS     = -g -Wall -Wextra -O3 -fsigned-char
ZIP_CFLAGS   = $(CXXFLAGS)
# Linker flags
BASE_LDFLAGS = -L$(LIBRARY) -lSDL2 -lXext -lXau -lXdmcp -lX11 -lGLESv2 -lEGL -lIMGegl -lsrv_um -lts

# Target compiler options
PREFIX   = $(PNDSDK)
TOOLS    = bin
TARGET   = arm-none-linux-gnueabi-
INCLUDE  = $(PREFIX)/usr/include
LIBRARY  = $(PREFIX)/usr/lib
NEON     = -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ffast-math -fsingle-precision-constant -fno-inline-functions
CXXFLAGS += $(NEON) -DUSE_GLES -DOGL_DIRECT_LINK -DPANDORA
LDFLAGS  = $(BASE_LDFLAGS)

# Assign includes
CXXFLAGS += -I. -Idosbox -I$(INCLUDE) -I$(INCLUDE)/SDL2

# Source files
SRCS       =  dosbox/dbopl.cpp                              \
	      3d_act1.cpp                                   \
	      3d_act2.cpp                                   \
	      3d_agent.cpp                                  \
	      3d_debug.cpp                                  \
	      3d_draw2.cpp                                  \
	      3d_draw.cpp                                   \
	      3d_game.cpp                                   \
	      3d_inter.cpp                                  \
	      3d_main.cpp                                   \
	      3d_menu.cpp                                   \
	      3d_msgs.cpp                                   \
	      3d_play.cpp                                   \
	      3d_scale.cpp                                  \
	      3d_state.cpp                                  \
	      bstone_adlib_decoder.cpp                      \
	      bstone_adlib_music_decoder.cpp                \
	      bstone_adlib_sfx_decoder.cpp                  \
	      bstone_audio_decoder.cpp                      \
	      bstone_audio_mixer.cpp                        \
	      bstone_binary_reader.cpp                      \
	      bstone_binary_writer.cpp                      \
	      bstone_c.cpp                                  \
	      bstone_file_stream.cpp                        \
	      bstone_istream.cpp                            \
	      bstone_memory_binary_reader.cpp               \
	      bstone_memory_stream.cpp                      \
	      bstone_ogl_api.cpp                            \
	      bstone_ogl_version.cpp                        \
	      bstone_opl2.cpp                               \
	      bstone_pcm_decoder.cpp                        \
	      colormap.cpp                                  \
	      d3.cpp                                        \
	      d3_d2.cpp                                     \
	      d3_dr2.cpp                                    \
	      id_ca.cpp                                     \
	      id_in.cpp                                     \
	      id_mm.cpp                                     \
	      id_pm.cpp                                     \
	      id_sd.cpp                                     \
	      id_sdd.cpp                                    \
	      id_us_1.cpp                                   \
	      id_us.cpp                                     \
	      id_vh.cpp                                     \
	      id_vl.cpp                                     \
	      jm_cio.cpp                                    \
	      jm_debug.cpp                                  \
	      jm_free.cpp                                   \
	      jm_io.cpp                                     \
	      jm_lzh.cpp                                    \
	      jm_tp.cpp                                     \
	      markhack.cpp                                  \
	      movie.cpp                                     \
	      scale.cpp                                     \
	      stub.cpp                                      \
	      vgapal.cpp

# Debug Configuration
ifeq ($(BUILDTYPE),debug)
CXXFLAGS += -DDEBUG
endif

# Assign paths to binaries/sources/objects
BUILD      = build
OBJDIR     = $(BUILD)/objs/$(BUILDTYPE)

OBJS       := $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o)) 

PROGRAM    := $(addprefix $(BUILD)/,$(PROGRAM)) 

# Assign Tools
CC  = $(PREFIX)/$(TOOLS)/$(TARGET)gcc
CXX = $(PREFIX)/$(TOOLS)/$(TARGET)g++
AR  = $(PREFIX)/$(TOOLS)/$(TARGET)ar

# Build rules
all: setup $(PROGRAM)

setup:
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/dosbox
	
$(PROGRAM): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(PROGRAM) $(OBJS)
 
