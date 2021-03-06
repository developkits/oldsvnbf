#CXXFLAGS= -ggdb3
CXXFLAGS= -O3 -fomit-frame-pointer
override CXXFLAGS+= -Wall -fsigned-char

PLATFORM= $(shell uname -s)
PLATFORM_SUFFIX= 

INCLUDES= -Ishared -Iengine -Igame -Ienet/include

STRIP=
ifeq (,$(findstring -g,$(CXXFLAGS)))
ifeq (,$(findstring -pg,$(CXXFLAGS)))
	STRIP=strip
endif
endif

MV=mv

ifneq (,$(findstring MINGW,$(PLATFORM)))
WINDRES= windres
CLIENT_INCLUDES= -DINTERFACE -DIRC $(INCLUDES) -Iinclude
CLIENT_LIBS= -mwindows -lmingw32 -Llib -lSDLmain -lSDL -lSDL_image -lSDL_mixer -lzdll -lopengl32 -lenet -lws2_32 -lwinmm
else
CLIENT_INCLUDES= -DINTERFACE -DIRC $(INCLUDES) -I/usr/X11R6/include `sdl-config --cflags`
CLIENT_LIBS= -Lenet/.libs -lenet -L/usr/X11R6/lib `sdl-config --libs` -lSDL_image -lSDL_mixer -lz -lGL
endif
ifeq ($(PLATFORM),Linux)
CLIENT_LIBS+= -lrt
endif
CLIENT_OBJS= \
	shared/crypto.o \
	shared/geom.o \
	shared/stream.o \
	shared/tools.o \
	shared/zip.o \
	engine/bih.o \
	engine/blend.o \
	engine/blob.o \
	engine/client.o \
	engine/command.o \
	engine/console.o \
	engine/decal.o \
	engine/dynlight.o \
	engine/glare.o \
	engine/grass.o \
	engine/irc.o	\
	engine/lightmap.o \
	engine/main.o \
	engine/material.o \
	engine/menus.o \
	engine/movie.o \
	engine/normal.o \
	engine/octa.o \
	engine/octaedit.o \
	engine/octarender.o \
	engine/physics.o \
	engine/pvs.o \
	engine/rendergl.o \
	engine/rendermodel.o \
	engine/renderparticles.o \
	engine/rendersky.o \
	engine/rendertext.o \
	engine/renderva.o \
	engine/server.o \
	engine/serverbrowser.o \
	engine/shader.o \
	engine/shadowmap.o \
	engine/sound.o \
	engine/texture.o \
	engine/ui.o \
	engine/water.o \
	engine/world.o \
	engine/worldio.o \
	game/ai.o \
	game/client.o \
	game/ctf.o \
	game/entities.o \
	game/game.o \
	game/hud.o \
	game/physics.o \
	game/projs.o \
	game/server.o \
	game/stf.o \
	game/weapons.o
	
CLIENT_PCH= shared/cube.h.gch engine/engine.h.gch game/game.h.gch

ifneq (,$(findstring MINGW,$(PLATFORM)))
SERVER_INCLUDES= -DSTANDALONE -DMASTERSERVER -DIRC $(INCLUDES) -Iinclude
SERVER_LIBS= -Llib -lzdll -lenet -lws2_32 -lwinmm
else
SERVER_INCLUDES= -DSTANDALONE -DMASTERSERVER -DIRC $(INCLUDES)
SERVER_LIBS= -Lenet/.libs -lenet -lz
endif
SERVER_OBJS= \
	shared/crypto-standalone.o \
	shared/stream-standalone.o \
	shared/tools-standalone.o \
	engine/command-standalone.o \
	engine/irc-standalone.o \
	engine/master-standalone.o \
	engine/server-standalone.o \
	game/server-standalone.o

default: all

enet/Makefile:
	cd enet; ./configure --enable-shared=no --enable-static=yes

libenet: enet/Makefile
	$(MAKE) -C enet/ all

clean-enet: enet/Makefile
	$(MAKE) -C enet/ clean

clean: clean-client clean-server

clean-client:
	@rm -fv $(CLIENT_PCH) $(CLIENT_OBJS) bfclient

clean-server:
	@rm -fv $(SERVER_OBJS) bfserver

%.h.gch: %.h
	$(CXX) $(CXXFLAGS) -o $@.tmp $(subst .h.gch,.h,$@)
	$(MV) $@.tmp $@

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.cpp,$@)

$(CLIENT_OBJS): CXXFLAGS += $(CLIENT_INCLUDES)
$(filter shared/%,$(CLIENT_OBJS)): $(filter shared/%,$(CLIENT_PCH))
$(filter engine/%,$(CLIENT_OBJS)): $(filter engine/%,$(CLIENT_PCH))
$(filter game/%,$(CLIENT_OBJS)): $(filter game/%,$(CLIENT_PCH))

$(SERVER_OBJS): CXXFLAGS += $(SERVER_INCLUDES)

ifneq (,$(findstring MINGW,$(PLATFORM)))
client: $(CLIENT_OBJS)
	$(WINDRES) -i bloodfrontier.rc -J rc -o bloodfrontier.res -O coff 
	$(CXX) $(CXXFLAGS) -o ../bin/bfclient.exe bloodfrontier.res $(CLIENT_OBJS) $(CLIENT_LIBS)

server: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o ../bin/bfserver.exe $(SERVER_OBJS) $(SERVER_LIBS)

install-client: client
ifneq (,$(STRIP))
	$(STRIP) ../bin/bfclient.exe
endif

install-server: server
ifneq (,$(STRIP))
	$(STRIP) ../bin/bfserver.exe
endif
else
client: libenet $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o bfclient $(CLIENT_OBJS) $(CLIENT_LIBS)
ifneq (,$(STRIP))
	$(STRIP) bfclient
endif

server: libenet $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o bfserver $(SERVER_OBJS) $(SERVER_LIBS)
ifneq (,$(STRIP))
	$(STRIP) bfserver
endif

install-client: client
	cp bfclient ../bin/bfclient$(PLATFORM_SUFFIX)

install-server: server
	cp bfserver ../bin/bfserver$(PLATFORM_SUFFIX)
endif

install: install-client install-server

depend:
	makedepend -Y -Ishared -Iengine -Igame $(subst .o,.cpp,$(CLIENT_OBJS))
	makedepend -a -o-standalone.o -Y -Ishared -Iengine -Igame $(subst -standalone.o,.cpp,$(SERVER_OBJS))
	makedepend -a -o.h.gch -Y -Ishared -Iengine -Igame $(subst .h.gch,.h,$(CLIENT_PCH))

all: client server
# DO NOT DELETE

shared/crypto.o: shared/cube.h shared/tools.h shared/command.h shared/geom.h
shared/crypto.o: shared/ents.h shared/iengine.h shared/igame.h
shared/geom.o: shared/cube.h shared/tools.h shared/command.h shared/geom.h
shared/geom.o: shared/ents.h shared/iengine.h shared/igame.h
shared/stream.o: shared/cube.h shared/tools.h shared/command.h shared/geom.h
shared/stream.o: shared/ents.h shared/iengine.h shared/igame.h
shared/tools.o: shared/cube.h shared/tools.h shared/command.h shared/geom.h
shared/tools.o: shared/ents.h shared/iengine.h shared/igame.h
shared/zip.o: shared/cube.h shared/tools.h shared/command.h shared/geom.h
shared/zip.o: shared/ents.h shared/iengine.h shared/igame.h
engine/bih.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/bih.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/bih.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/bih.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/blend.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/blend.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/blend.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/blend.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/blob.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/blob.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/blob.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/blob.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/client.o: engine/engine.h shared/cube.h shared/tools.h
engine/client.o: shared/command.h shared/geom.h shared/ents.h
engine/client.o: shared/iengine.h shared/igame.h engine/sound.h
engine/client.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/client.o: engine/texture.h engine/model.h engine/varray.h
engine/command.o: engine/engine.h shared/cube.h shared/tools.h
engine/command.o: shared/command.h shared/geom.h shared/ents.h
engine/command.o: shared/iengine.h shared/igame.h engine/sound.h
engine/command.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/command.o: engine/texture.h engine/model.h engine/varray.h
engine/console.o: engine/engine.h shared/cube.h shared/tools.h
engine/console.o: shared/command.h shared/geom.h shared/ents.h
engine/console.o: shared/iengine.h shared/igame.h engine/sound.h
engine/console.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/console.o: engine/texture.h engine/model.h engine/varray.h
engine/decal.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/decal.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/decal.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/decal.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/dynlight.o: engine/engine.h shared/cube.h shared/tools.h
engine/dynlight.o: shared/command.h shared/geom.h shared/ents.h
engine/dynlight.o: shared/iengine.h shared/igame.h engine/sound.h
engine/dynlight.o: engine/world.h engine/octa.h engine/lightmap.h
engine/dynlight.o: engine/bih.h engine/texture.h engine/model.h
engine/dynlight.o: engine/varray.h
engine/glare.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/glare.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/glare.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/glare.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/glare.o: engine/rendertarget.h
engine/grass.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/grass.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/grass.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/grass.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/lightmap.o: engine/engine.h shared/cube.h shared/tools.h
engine/lightmap.o: shared/command.h shared/geom.h shared/ents.h
engine/lightmap.o: shared/iengine.h shared/igame.h engine/sound.h
engine/lightmap.o: engine/world.h engine/octa.h engine/lightmap.h
engine/lightmap.o: engine/bih.h engine/texture.h engine/model.h
engine/lightmap.o: engine/varray.h
engine/main.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/main.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/main.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/main.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/material.o: engine/engine.h shared/cube.h shared/tools.h
engine/material.o: shared/command.h shared/geom.h shared/ents.h
engine/material.o: shared/iengine.h shared/igame.h engine/sound.h
engine/material.o: engine/world.h engine/octa.h engine/lightmap.h
engine/material.o: engine/bih.h engine/texture.h engine/model.h
engine/material.o: engine/varray.h
engine/menus.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/menus.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/menus.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/menus.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/movie.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/movie.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/movie.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/movie.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/normal.o: engine/engine.h shared/cube.h shared/tools.h
engine/normal.o: shared/command.h shared/geom.h shared/ents.h
engine/normal.o: shared/iengine.h shared/igame.h engine/sound.h
engine/normal.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/normal.o: engine/texture.h engine/model.h engine/varray.h
engine/octa.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/octa.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/octa.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/octa.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/octaedit.o: engine/engine.h shared/cube.h shared/tools.h
engine/octaedit.o: shared/command.h shared/geom.h shared/ents.h
engine/octaedit.o: shared/iengine.h shared/igame.h engine/sound.h
engine/octaedit.o: engine/world.h engine/octa.h engine/lightmap.h
engine/octaedit.o: engine/bih.h engine/texture.h engine/model.h
engine/octaedit.o: engine/varray.h
engine/octarender.o: engine/engine.h shared/cube.h shared/tools.h
engine/octarender.o: shared/command.h shared/geom.h shared/ents.h
engine/octarender.o: shared/iengine.h shared/igame.h engine/sound.h
engine/octarender.o: engine/world.h engine/octa.h engine/lightmap.h
engine/octarender.o: engine/bih.h engine/texture.h engine/model.h
engine/octarender.o: engine/varray.h
engine/physics.o: engine/engine.h shared/cube.h shared/tools.h
engine/physics.o: shared/command.h shared/geom.h shared/ents.h
engine/physics.o: shared/iengine.h shared/igame.h engine/sound.h
engine/physics.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/physics.o: engine/texture.h engine/model.h engine/varray.h
engine/physics.o: engine/mpr.h
engine/pvs.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/pvs.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/pvs.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/pvs.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/rendergl.o: engine/engine.h shared/cube.h shared/tools.h
engine/rendergl.o: shared/command.h shared/geom.h shared/ents.h
engine/rendergl.o: shared/iengine.h shared/igame.h engine/sound.h
engine/rendergl.o: engine/world.h engine/octa.h engine/lightmap.h
engine/rendergl.o: engine/bih.h engine/texture.h engine/model.h
engine/rendergl.o: engine/varray.h
engine/rendermodel.o: engine/engine.h shared/cube.h shared/tools.h
engine/rendermodel.o: shared/command.h shared/geom.h shared/ents.h
engine/rendermodel.o: shared/iengine.h shared/igame.h engine/sound.h
engine/rendermodel.o: engine/world.h engine/octa.h engine/lightmap.h
engine/rendermodel.o: engine/bih.h engine/texture.h engine/model.h
engine/rendermodel.o: engine/varray.h engine/ragdoll.h engine/animmodel.h
engine/rendermodel.o: engine/vertmodel.h engine/skelmodel.h engine/md2.h
engine/rendermodel.o: engine/md3.h engine/md5.h engine/obj.h engine/smd.h
engine/rendermodel.o: engine/iqm.h
engine/renderparticles.o: engine/engine.h shared/cube.h shared/tools.h
engine/renderparticles.o: shared/command.h shared/geom.h shared/ents.h
engine/renderparticles.o: shared/iengine.h shared/igame.h engine/sound.h
engine/renderparticles.o: engine/world.h engine/octa.h engine/lightmap.h
engine/renderparticles.o: engine/bih.h engine/texture.h engine/model.h
engine/renderparticles.o: engine/varray.h engine/rendertarget.h
engine/renderparticles.o: engine/depthfx.h engine/lensflare.h
engine/renderparticles.o: engine/explosion.h engine/lightning.h
engine/rendersky.o: engine/engine.h shared/cube.h shared/tools.h
engine/rendersky.o: shared/command.h shared/geom.h shared/ents.h
engine/rendersky.o: shared/iengine.h shared/igame.h engine/sound.h
engine/rendersky.o: engine/world.h engine/octa.h engine/lightmap.h
engine/rendersky.o: engine/bih.h engine/texture.h engine/model.h
engine/rendersky.o: engine/varray.h
engine/rendertext.o: engine/engine.h shared/cube.h shared/tools.h
engine/rendertext.o: shared/command.h shared/geom.h shared/ents.h
engine/rendertext.o: shared/iengine.h shared/igame.h engine/sound.h
engine/rendertext.o: engine/world.h engine/octa.h engine/lightmap.h
engine/rendertext.o: engine/bih.h engine/texture.h engine/model.h
engine/rendertext.o: engine/varray.h
engine/renderva.o: engine/engine.h shared/cube.h shared/tools.h
engine/renderva.o: shared/command.h shared/geom.h shared/ents.h
engine/renderva.o: shared/iengine.h shared/igame.h engine/sound.h
engine/renderva.o: engine/world.h engine/octa.h engine/lightmap.h
engine/renderva.o: engine/bih.h engine/texture.h engine/model.h
engine/renderva.o: engine/varray.h
engine/server.o: engine/engine.h shared/cube.h shared/tools.h
engine/server.o: shared/command.h shared/geom.h shared/ents.h
engine/server.o: shared/iengine.h shared/igame.h engine/sound.h
engine/server.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/server.o: engine/texture.h engine/model.h engine/varray.h
engine/serverbrowser.o: engine/engine.h shared/cube.h shared/tools.h
engine/serverbrowser.o: shared/command.h shared/geom.h shared/ents.h
engine/serverbrowser.o: shared/iengine.h shared/igame.h engine/sound.h
engine/serverbrowser.o: engine/world.h engine/octa.h engine/lightmap.h
engine/serverbrowser.o: engine/bih.h engine/texture.h engine/model.h
engine/serverbrowser.o: engine/varray.h
engine/shader.o: engine/engine.h shared/cube.h shared/tools.h
engine/shader.o: shared/command.h shared/geom.h shared/ents.h
engine/shader.o: shared/iengine.h shared/igame.h engine/sound.h
engine/shader.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/shader.o: engine/texture.h engine/model.h engine/varray.h
engine/shadowmap.o: engine/engine.h shared/cube.h shared/tools.h
engine/shadowmap.o: shared/command.h shared/geom.h shared/ents.h
engine/shadowmap.o: shared/iengine.h shared/igame.h engine/sound.h
engine/shadowmap.o: engine/world.h engine/octa.h engine/lightmap.h
engine/shadowmap.o: engine/bih.h engine/texture.h engine/model.h
engine/shadowmap.o: engine/varray.h engine/rendertarget.h
engine/sound.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/sound.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/sound.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/sound.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/texture.o: engine/engine.h shared/cube.h shared/tools.h
engine/texture.o: shared/command.h shared/geom.h shared/ents.h
engine/texture.o: shared/iengine.h shared/igame.h engine/sound.h
engine/texture.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/texture.o: engine/texture.h engine/model.h engine/varray.h
engine/texture.o: engine/scale.h
engine/ui.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/ui.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/ui.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/ui.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/ui.o: engine/textedit.h
engine/water.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/water.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/water.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/water.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/world.o: engine/engine.h shared/cube.h shared/tools.h shared/command.h
engine/world.o: shared/geom.h shared/ents.h shared/iengine.h shared/igame.h
engine/world.o: engine/sound.h engine/world.h engine/octa.h engine/lightmap.h
engine/world.o: engine/bih.h engine/texture.h engine/model.h engine/varray.h
engine/worldio.o: engine/engine.h shared/cube.h shared/tools.h
engine/worldio.o: shared/command.h shared/geom.h shared/ents.h
engine/worldio.o: shared/iengine.h shared/igame.h engine/sound.h
engine/worldio.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
engine/worldio.o: engine/texture.h engine/model.h engine/varray.h
game/ai.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/ai.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/ai.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/ai.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/ai.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/ai.o: game/scoreboard.h game/weapons.h
game/client.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/client.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/client.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/client.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/client.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/client.o: game/scoreboard.h game/weapons.h
game/ctf.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/ctf.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/ctf.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/ctf.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/ctf.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/ctf.o: game/scoreboard.h game/weapons.h
game/entities.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/entities.o: shared/command.h shared/geom.h shared/ents.h
game/entities.o: shared/iengine.h shared/igame.h engine/sound.h
game/entities.o: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
game/entities.o: engine/texture.h engine/model.h engine/varray.h game/ai.h
game/entities.o: game/ctf.h game/stf.h game/vars.h game/scoreboard.h game/weapons.h
game/game.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/game.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/game.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/game.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/game.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/game.o: game/scoreboard.h game/weapons.h
game/hud.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/hud.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/hud.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/hud.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/hud.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/hud.o: game/scoreboard.h game/compass.h game/weapons.h
game/physics.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/physics.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/physics.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/physics.o: engine/lightmap.h engine/bih.h engine/texture.h
game/physics.o: engine/model.h engine/varray.h game/ai.h game/ctf.h
game/physics.o: game/stf.h game/vars.h game/scoreboard.h game/weapons.h
game/projs.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/projs.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/projs.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/projs.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/projs.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/projs.o: game/scoreboard.h game/weapons.h
game/server.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/server.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/server.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/server.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/server.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/server.o: game/scoreboard.h game/stfmode.h game/ctfmode.h game/duelmut.h
game/server.o: game/aiman.h game/auth.h game/extinfo.h game/weapons.h
game/stf.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/stf.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/stf.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/stf.o: engine/lightmap.h engine/bih.h engine/texture.h engine/model.h
game/stf.o: engine/varray.h game/ai.h game/ctf.h game/stf.h game/vars.h
game/stf.o: game/scoreboard.h game/weapons.h
game/weapons.o: game/game.h engine/engine.h shared/cube.h shared/tools.h
game/weapons.o: shared/command.h shared/geom.h shared/ents.h shared/iengine.h
game/weapons.o: shared/igame.h engine/sound.h engine/world.h engine/octa.h
game/weapons.o: engine/lightmap.h engine/bih.h engine/texture.h
game/weapons.o: engine/model.h engine/varray.h game/ai.h game/ctf.h
game/weapons.o: game/stf.h game/vars.h game/scoreboard.h game/weapons.h

shared/crypto-standalone.o: shared/cube.h shared/tools.h shared/command.h
shared/crypto-standalone.o: shared/geom.h shared/ents.h shared/iengine.h
shared/crypto-standalone.o: shared/igame.h
shared/stream-standalone.o: shared/cube.h shared/tools.h shared/command.h
shared/stream-standalone.o: shared/geom.h shared/ents.h shared/iengine.h
shared/stream-standalone.o: shared/igame.h
shared/tools-standalone.o: shared/cube.h shared/tools.h shared/command.h
shared/tools-standalone.o: shared/geom.h shared/ents.h shared/iengine.h
shared/tools-standalone.o: shared/igame.h
engine/command-standalone.o: engine/engine.h shared/cube.h shared/tools.h
engine/command-standalone.o: shared/command.h shared/geom.h shared/ents.h
engine/command-standalone.o: shared/iengine.h shared/igame.h engine/sound.h
engine/command-standalone.o: engine/world.h engine/octa.h engine/lightmap.h
engine/command-standalone.o: engine/bih.h engine/texture.h engine/model.h
engine/command-standalone.o: engine/varray.h
engine/master-standalone.o: engine/engine.h shared/cube.h shared/tools.h
engine/master-standalone.o: shared/command.h shared/geom.h shared/ents.h
engine/master-standalone.o: shared/iengine.h shared/igame.h engine/sound.h
engine/master-standalone.o: engine/world.h engine/octa.h engine/lightmap.h
engine/master-standalone.o: engine/bih.h engine/texture.h engine/model.h
engine/master-standalone.o: engine/varray.h
engine/server-standalone.o: engine/engine.h shared/cube.h shared/tools.h
engine/server-standalone.o: shared/command.h shared/geom.h shared/ents.h
engine/server-standalone.o: shared/iengine.h shared/igame.h engine/sound.h
engine/server-standalone.o: engine/world.h engine/octa.h engine/lightmap.h
engine/server-standalone.o: engine/bih.h engine/texture.h engine/model.h
engine/server-standalone.o: engine/varray.h
game/server-standalone.o: game/game.h engine/engine.h shared/cube.h
game/server-standalone.o: shared/tools.h shared/command.h shared/geom.h
game/server-standalone.o: shared/ents.h shared/iengine.h shared/igame.h
game/server-standalone.o: engine/sound.h engine/world.h engine/octa.h
game/server-standalone.o: engine/lightmap.h engine/bih.h engine/texture.h
game/server-standalone.o: engine/model.h engine/varray.h game/ai.h game/ctf.h
game/server-standalone.o: game/stf.h game/vars.h game/stfmode.h
game/server-standalone.o: game/ctfmode.h game/duelmut.h game/aiman.h
game/server-standalone.o: game/auth.h game/extinfo.h game/weapons.h

shared/cube.h.gch: shared/tools.h shared/command.h shared/geom.h
shared/cube.h.gch: shared/ents.h shared/iengine.h shared/igame.h
engine/engine.h.gch: shared/cube.h shared/tools.h shared/command.h
engine/engine.h.gch: shared/geom.h shared/ents.h shared/iengine.h
engine/engine.h.gch: shared/igame.h engine/sound.h engine/world.h
engine/engine.h.gch: engine/octa.h engine/lightmap.h engine/bih.h
engine/engine.h.gch: engine/texture.h engine/model.h engine/varray.h
game/game.h.gch: engine/engine.h shared/cube.h shared/tools.h
game/game.h.gch: shared/command.h shared/geom.h shared/ents.h
game/game.h.gch: shared/iengine.h shared/igame.h engine/sound.h
game/game.h.gch: engine/world.h engine/octa.h engine/lightmap.h engine/bih.h
game/game.h.gch: engine/texture.h engine/model.h engine/varray.h game/ai.h
game/game.h.gch: game/ctf.h game/stf.h game/vars.h game/scoreboard.h game/weapons.h
