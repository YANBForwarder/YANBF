BUILD		:= build

ifneq ($(BUILD),$(notdir $(CURDIR)))

TARGET := generator

SOURCES		+= source
INCLUDES	+= 

#--- set toolchain name
ifneq ($(CLANG),)
export CC	:=	$(PREFIX)clang
#export CXX	:=	$(PREFIX)clang++ -stdlib=libc++
export CXX	:=	$(PREFIX)clang++
else ifneq ($(OPEN64),)
export CC       :=      $(PREFIX)opencc
export CXX      :=      $(PREFIX)openCC
else
export CC	:=	$(PREFIX)gcc
export CXX	:=	$(PREFIX)g++
endif

#export AS	:=	$(PREFIX)as
#export AR	:=	$(PREFIX)ar
#export OBJCOPY	:=	$(PREFIX)objcopy
export STRIP	:=	$(PREFIX)strip

#--- set path
### relative path from build
ifeq ($(OS),Windows_NT)
LIBS	+=	-Wl,-Bstatic -lMagick++-7.Q16HDRI -lMagickCore-7.Q16HDRI -lMagickWand-7.Q16HDRI
LIBDIRS	:= /lib
else
LIBS	+= -Wl,-Bstatic -lMagick++-7.Q16HDRI -lMagickCore-7.Q16HDRI -lMagickWand-7.Q16HDRI -Wl,-Bdynamic -ldl
LIBDIRS	:= libs
endif
export LIBS
export LIBDIRS

ifeq ($(OS),Windows_NT)
export SUFF	:=	.exe
endif

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

#--- flags
#ARCH	:=	
CFLAGS	:=	-Wall -Wno-parentheses \
			-W -Wno-sign-compare -Wno-unused-parameter -Wno-unused-value \
			-O2 -fomit-frame-pointer -ffast-math\
			-DNO_VIZ \
			$(ARCH)
#-DNO_VIZ for zlib.

CFLAGS	+=	$(INCLUDE)
CXXFLAGS	:=	`Magick++-config --cppflags --cxxflags` $(CFLAGS) -std=gnu++17 `Magick++-config --ldflags --libs`
#lovely hack...
#bah -Wno-pointer-sign must be stripped for iPodLinux
# CFLAGS	+=  -Wno-pointer-sign -std=gnu99

ASFLAGS	:=	$(ARCH)
LDFLAGS	=	$(ARCH) $(LDF) -O2
#-Wl,-Map,$(notdir $*.map)


CXXFLAGS += -fno-rtti
LDFLAGS += -static -static-libgcc -static-libstdc++




export CFLAGS
export CXXFLAGS
export LDFLAGS
 
export BIN	:=	$(CURDIR)/$(TARGET)$(SUFF)
export DEPSDIR := $(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
 
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	ifneq ($(USTL),)
		export LD	:=	$(CC)
	else
		#gcc doesn't understand -static-libstdc++...
		export LD	:=	$(CXX)
	endif
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

.PHONY: $(BUILD) clean
 
$(BUILD):
	
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile -j8

clean:
	@#echo clean ...
	@rm -fr $(BUILD) 
	@rm $(TARGET)

#---------------------------------------------------------------------------------
else

#%.a:
#	@#echo $(notdir $@)
#	@rm -f $@
#	@$(AR) -rc $@ $^

%.o: %.cpp
	@#echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@ $(ERROR_FILTER)

%.o: %.c
	@#echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

%.o: %.s
	@#echo $(notdir $<)
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d -x assembler-with-cpp $(ASFLAGS) -c $< -o $@ $(ERROR_FILTER)

DEPENDS	:=	$(OFILES:.o=.d)

$(BIN): $(OFILES)
	@#echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@
	@$(STRIP) $@

-include $(DEPENDS)

endif
