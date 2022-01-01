
# ----------------------------------------------------------------------------------

DependPath=_Depend

CFLAGS+= -O3 -G3 -Wall
CFLAGS+= -MMD -MP -MF $(DependPath)/$(notdir $*.d)
CXXFLAGS= $(CFLAGS) -fno-exceptions
ASFLAGS= $(CFLAGS)

$(LibName).a: $(DependPath) $(OBJS)
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCDIR) -c -o $*.o $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIR) -c -o $*.o $<

$(DependPath):
	-mkdir $(DependPath)

clean:
	-rm -f $(LibName).a $(OBJS) $(wildcard $(DependPath)/*.d)
	-rmdir $(DependPath)

-include $(wildcard $(DependPath)/*.d)

