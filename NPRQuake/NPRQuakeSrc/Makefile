#
# Quake Makefile for Linux 2.0
#
# Aug '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#

BASEVERSION=1.09
VERSION=$(BASEVERSION)$(GLIBC)

ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6 ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif

ifneq (,$(findstring alpha,$(shell uname -m)))
ARCH=axp
else
ARCH=i386
endif
NOARCH=noarch

MESA_DIR=/usr/local/src/Mesa-2.6

BUILD_DEBUG_DIR=debug$(ARCH)$(GLIBC)
BUILD_RELEASE_DIR=release$(ARCH)$(GLIBC)

CC=gcc

RELEASE_CFLAGS=$(BASE_CFLAGS) -mcpu=i586 -march=i586 -O6 -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
LDFLAGS=-lm
CFLAGS=$(RELEASE_CFLAGS)

MESAGLLDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -L$(MESA_DIR)/lib -lMesaGL -lglide2x -lX11 -lXext -ldl
TDFXGLLDFLAGS=-L$(TDFXGL_DIR)/release$(ARCH)$(GLIBC) -l3dfxgl -lglide2x -ldl
GLLDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -lGL -lX11 -lXext -ldl -lXxf86dga -lXxf86vm -lm `sdl-config --libs`
GLCFLAGS=-DGLQUAKE -I$(MESA_DIR)/include -I/usr/include/glide `sdl-config --cflags`

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) -o $@ -c $<
DO_GL_CC=$(CC) $(CFLAGS) $(GLCFLAGS) -o $@ -c $<
DO_GL_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) $(GLCFLAGS) -o $@ -c $<
DO_O_CC=$(CC) -O $(CFLAGS) -o $@ -c $<
DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
DO_GL_AS=$(CC) $(CFLAGS) $(GLCFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

TARGETS=$(BUILDDIR)/bin/glquake.sdl

default:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/bin \
		$(BUILD_DEBUG_DIR)/glquake
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(CFLAGS)"

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/bin \
		$(BUILD_DEBUG_DIR)/glquake
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-mkdir $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/bin \
		$(BUILD_RELEASE_DIR)/glquake
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)
	cp $(TARGETS) ../build

#############################################################################
# GLQuake
#############################################################################

GLQUAKE_OBJS= \
	$(BUILDDIR)/glquake/cl_demo.o \
	$(BUILDDIR)/glquake/cl_input.o \
	$(BUILDDIR)/glquake/cl_main.o \
	$(BUILDDIR)/glquake/cl_parse.o \
	$(BUILDDIR)/glquake/cl_tent.o \
	$(BUILDDIR)/glquake/chase.o \
	$(BUILDDIR)/glquake/cmd.o \
	$(BUILDDIR)/glquake/common.o \
	$(BUILDDIR)/glquake/console.o \
	$(BUILDDIR)/glquake/crc.o \
	$(BUILDDIR)/glquake/cvar.o \
	\
	$(BUILDDIR)/glquake/gl_draw.o \
	$(BUILDDIR)/glquake/gl_mesh.o \
	$(BUILDDIR)/glquake/gl_model.o \
	$(BUILDDIR)/glquake/gl_refrag.o \
	$(BUILDDIR)/glquake/gl_rlight.o \
	$(BUILDDIR)/glquake/gl_rmain.o \
	$(BUILDDIR)/glquake/gl_rmisc.o \
	$(BUILDDIR)/glquake/gl_rsurf.o \
	$(BUILDDIR)/glquake/gl_screen.o \
	$(BUILDDIR)/glquake/gl_test.o \
	$(BUILDDIR)/glquake/gl_warp.o \
	\
	$(BUILDDIR)/glquake/host.o \
	$(BUILDDIR)/glquake/host_cmd.o \
	$(BUILDDIR)/glquake/keys.o \
	$(BUILDDIR)/glquake/menu.o \
	$(BUILDDIR)/glquake/mathlib.o \
	$(BUILDDIR)/glquake/net_dgrm.o \
	$(BUILDDIR)/glquake/net_loop.o \
	$(BUILDDIR)/glquake/net_main.o \
	$(BUILDDIR)/glquake/net_vcr.o \
	$(BUILDDIR)/glquake/net_udp.o \
	$(BUILDDIR)/glquake/net_bsd.o \
	$(BUILDDIR)/glquake/pr_cmds.o \
	$(BUILDDIR)/glquake/pr_edict.o \
	$(BUILDDIR)/glquake/pr_exec.o \
	$(BUILDDIR)/glquake/r_part.o \
	$(BUILDDIR)/glquake/sbar.o \
	$(BUILDDIR)/glquake/sv_main.o \
	$(BUILDDIR)/glquake/sv_phys.o \
	$(BUILDDIR)/glquake/sv_move.o \
	$(BUILDDIR)/glquake/sv_user.o \
	$(BUILDDIR)/glquake/zone.o	\
	$(BUILDDIR)/glquake/view.o	\
	$(BUILDDIR)/glquake/wad.o \
	$(BUILDDIR)/glquake/world.o \
	$(BUILDDIR)/glquake/cd_linux.o \
	$(BUILDDIR)/glquake/sys_linux.o \
	$(BUILDDIR)/glquake/snd_dma.o \
	$(BUILDDIR)/glquake/snd_mem.o \
	$(BUILDDIR)/glquake/snd_mix.o \
	$(BUILDDIR)/glquake/snd_linux.o \
	\
	$(BUILDDIR)/glquake/math.o \
	$(BUILDDIR)/glquake/worlda.o \
	$(BUILDDIR)/glquake/snd_mixa.o \
	$(BUILDDIR)/glquake/sys_dosa.o

GLSVGA_OBJS=$(BUILDDIR)/glquake/gl_vidlinux.o

GLX_OBJS=$(BUILDDIR)/glquake/gl_vidlinuxSDL.o

$(BUILDDIR)/bin/glquake : $(GLQUAKE_OBJS) $(GLSVGA_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQUAKE_OBJS) $(GLSVGA_OBJS) $(MESAGLLDFLAGS) $(SVGALDFLAGS) $(LDFLAGS)

$(BUILDDIR)/bin/glquake.sdl : $(GLQUAKE_OBJS) $(GLX_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQUAKE_OBJS) $(GLX_OBJS) $(GLLDFLAGS) $(LDFLAGS)

$(BUILDDIR)/bin/glquake.3dfxgl : $(GLQUAKE_OBJS) $(GLSVGA_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQUAKE_OBJS) $(GLSVGA_OBJS) $(TDFXGLLDFLAGS) $(SVGALDFLAGS) $(LDFLAGS)

$(BUILDDIR)/glquake/cl_demo.o :      cl_demo.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_input.o :     cl_input.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_main.o :      cl_main.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_parse.o :     cl_parse.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_tent.o :      cl_tent.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/chase.o :        chase.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cmd.o :          cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/common.o :       common.c
	$(DO_GL_DEBUG_CC)

$(BUILDDIR)/glquake/console.o :      console.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/crc.o :          crc.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cvar.o :         cvar.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_draw.o :      gl_draw.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_mesh.o :      gl_mesh.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_model.o :     gl_model.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_refrag.o :    gl_refrag.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rlight.o :    gl_rlight.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rmain.o :     gl_rmain.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rmisc.o :     gl_rmisc.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rsurf.o :     gl_rsurf.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_screen.o :    gl_screen.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_test.o :      gl_test.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_vidlinux.o :  gl_vidlinux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_vidlinuxSDL.o :  gl_vidlinuxSDL.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_warp.o :      gl_warp.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/host.o :         host.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/host_cmd.o :     host_cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/keys.o :         keys.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/menu.o :         menu.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/mathlib.o :      mathlib.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_dgrm.o :     net_dgrm.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_loop.o :     net_loop.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_main.o :     net_main.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_vcr.o :      net_vcr.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_udp.o :      net_udp.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_bsd.o :      net_bsd.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/pr_cmds.o :      pr_cmds.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/pr_edict.o :     pr_edict.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/pr_exec.o :      pr_exec.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/r_part.o :       r_part.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sbar.o :         sbar.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_main.o :      sv_main.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_phys.o :      sv_phys.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_move.o :      sv_move.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_user.o :      sv_user.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/zone.o	:        zone.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/view.o	:        view.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/wad.o :          wad.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/world.o :        world.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cd_linux.o :     cd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sys_linux.o :    sys_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_dma.o :      snd_dma.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_mem.o :      snd_mem.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_mix.o :      snd_mix.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_linux.o :    snd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/math.o :         math.s
	$(DO_GL_AS)

$(BUILDDIR)/glquake/worlda.o :       worlda.s
	$(DO_GL_AS)

$(BUILDDIR)/glquake/snd_mixa.o :     snd_mixa.s
	$(DO_GL_AS)

$(BUILDDIR)/glquake/sys_dosa.o :     sys_dosa.s
	$(DO_GL_AS)

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release
	rm -f squake.spec glquake.spec quake.x11.spec

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(GLQUAKE_OBJS) $(GLSVGA_OBJS) $(GLX_OBJS)

