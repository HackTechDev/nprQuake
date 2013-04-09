RELEASE_CFLAGS=-Dstricmp=strcasecmp -mcpu=i586 -march=i586 -O6 -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations

DEBUG_CFLAGS=-Dstricmp=strcasecmp -g

CFLAGS=$(RELEASE_CFLAGS)

DR_FLAGS=-DGLQUAKE -D_DLL_BUILD -shared

renderers=build/dr_default.so build/brush.so build/bprint.so build/sketch.so

release:
	make all CFLAGS="$(RELEASE_CFLAGS)"

debug:
	make all CFLAGS="$(DEBUG_CFLAGS)"

all: quake dynamic_renderers

quake:
	make -C NPRQuakeSrc build_release CFLAGS="$(CFLAGS)"

dynamic_renderers: $(renderers)

$(renderers): build/%.so: dynamic_r/%.c dynamic_r/*.h
	cd dynamic_r
	gcc $(CFLAGS) $(DR_FLAGS) -o $@ $<

clean:
	make -C NPRQuakeSrc clean
