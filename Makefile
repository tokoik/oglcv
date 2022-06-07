TARGET	= oglcv
SOURCES	= $(wildcard *.cpp) $(wildcard libs/ImGui/imgui*.cpp libs/ImGui/nfd_gtk.cpp)
HEADERS	= $(wildcard *.h) $(wildcard libs/include/*.h)
OBJECTS	= $(patsubst %.cpp,%.o,$(SOURCES))
CXXFLAGS	= --std=c++17 -g -Wall -DDEBUG -DX11 -Ilibs/include
LDLIBS	= -ldl `pkg-config glfw3 --libs` `pkg-config gtk+-3.0 --libs`

.PHONY: clean

$(TARGET): $(OBJECTS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(TARGET).dep: $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -MM $(SOURCES) > $@

clean:
	-$(RM) $(TARGET) *.o lib/*.o *~ .*~ *.bak *.dep imgui.ini a.out core

-include $(TARGET).dep
