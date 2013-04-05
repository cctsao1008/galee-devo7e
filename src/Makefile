############################################
#隐藏编译详细过程
$(VERBOSE).SILENT:

############################################
#禁用'内建规则'
MAKEFLAGS += --no-builtin-rules

############################################
#设定目标参数
.SUFFIXES:
TARGET   	?= devo7e
HGVERSION	:= ${TARGET}-Galee
LINKFILE 	?= $(TARGET).ld
EXEEXT   	?= elf
DFUVERSION 	?= 2
CRC_OFFSET	:= 8192
DFU_ARGS	:= -c 7 -b 0x08003000

############################################
#设定工具链
CROSS    ?= arm-none-eabi-
    CC   = $(CROSS)gcc
    CXX  = $(CROSS)g++
    LD   = $(CROSS)ld
    AR   = $(CROSS)ar
    AS   = $(CROSS)as
    CP   = $(CROSS)objcopy
    DUMP = $(CROSS)objdump
    NM   = $(CROSS)nm

############################################
#设定源码列表
SRCS   =	$(wildcard common/*.c)\
			$(wildcard common/msc2/*.c)\
			$(wildcard common/msc2/lib/*.c)\
			$(wildcard *.c)  
            
LIBOPENCM3 = common/libopencm3_stm32f1.a

############################################
#设定源码列表
ODIR     = objs
OBJS 	 = $(addprefix $(ODIR)/, $(notdir $(SRCS:.c=.o) ))

############################################
#HGTEST
# Rebuild on hg version change.
# Based on http://mercurial.selenic.com/wiki/VersioningWithMake
# We need to always run this code, as opposed to running it from a prerequisite
# The HGTEST variable is not used
#HGTEST := $(shell mkdir -p $(ODIR); \
#	[ -f $(ODIR)/hgstamp ] || touch $(ODIR)/hgstamp; \
#	echo $(HGVERSION) | cmp -s $(ODIR)/hgstamp - \
#	|| echo $(HGVERSION) > $(ODIR)/hgstamp)

############################################
#设定编译选项
CFLAGS   =	-D"assert_param(x)=" -DSTM32F10X_HD -DSTM32F1 -mcpu=cortex-m3 \
			-mthumb -mfix-cortex-m3-ldrd -fdata-sections -ffunction-sections \
			-I. -Icommon -Icommon/msc2/lib -Icommon/msc2 -Icommon/include \
			-fno-builtin-printf -Os -Wall -Wextra \
			-std=gnu99 -DHGVERSION="\"${HGVERSION}\""

LFLAGS   	= -nostartfiles -Wl,-gc-sections -Wl,-Map=$(TARGET).map,--cref -lc
LFLAGS2  	= -Wl,-T$(LINKFILE)
LFLAGS2OPT  = -Wl,-T$(TARGET)_opt.ld

############################################
#编译目标文件
all : $(TARGET).dfu

############################################
#如果Makefile有变化，重新编译
$(OBJS) : Makefile 

############################################
#清除命令
.PHONY: clean 
clean:
	rm -f $(TARGET).$(EXEEXT) $(TARGET).bin $(TARGET).dfu $(TARGET).list \
		$(TARGET).map $(ODIR)/*.o $(ODIR)/*.P  $(ODIR)/*.bin
	@echo " + Clean finished."
	
############################################
#创建目标目录
$(OBJS): | $(ODIR)

$(ODIR):
	@mkdir -p $@

############################################
#生成固件
$(TARGET).$(EXEEXT): $(LINKFILE) $(OBJS) $(LIBOPENCM3)
	@echo " + Building '$@'"
	/bin/rm -f objs/optimize.ld
	echo "" > objs/optimize.ld	#Create an empty 'obj/optimize.ld' just in case the linker script needs it
	$(CC) -o $@ $(OBJS) $(LIBOPENCM3) $(LFLAGS) $(LFLAGS2) $(CFLAGS)

############################################
#.o 文件生成规则
## The autodependency magic below was adapeted from:
## http://mad-scientist.net/make/autodep.html
-include $(OBJS:.o=.P)
-include $(PROTO_OBJS:.o=.P)
-include $(PROTO_EXTRA_OBJS:.o=.P)

dollar = $$
define define_compile_rules
$(ODIR)/%.o: $(1)%.c
	@echo " + Compiling '$$<'"
	$(CC) $$(CFLAGS) -MD -c -o $$@ $$<
	@cp $(ODIR)/$$*.d $(ODIR)/$$*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$(dollar)//' \
                -e '/^$$(dollar)/ d' -e 's/$$(dollar)/ :/' < $(ODIR)/$$*.d >> $(ODIR)/$$*.P; \
            rm -f $(ODIR)/$$*.d

$(ODIR)/%.o: $(1)%.cpp
	@echo " + Compiling '$$<'"
	$(CXX) $$(CXXFLAGS) -MD -c -o $$@ $$<
	@cp $(ODIR)/$$*.d $(ODIR)/$$*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$(dollar)//' \
                -e '/^$$(dollar)/ d' -e 's/$$(dollar)/ :/' < $(ODIR)/$$*.d >> $(ODIR)/$$*.P; \
            rm -f $(ODIR)/$$*.d

endef
$(foreach directory,$(sort $(dir $(SRCS) $(PROTO_EXTRA_C))),$(eval $(call define_compile_rules,$(directory))))

############################################
#hg 版本变化时重新编译
# Rebuild on hg version change.
#$(ODIR)/version.o: $(ODIR)/hgstamp
#
#$(ODIR)/hgstamp:
#	echo $(HGVERSION) > $(ODIR)/hgstamp

############################################
#优化目标文件，缩减尺寸
$(TARGET).bin: $(TARGET).elf
	@echo " + Optimizing placement and re-linking"
	../utils/repack_ld.pl -mapfile $(TARGET).map -size $(CRC_OFFSET) > objs/optimize.ld
	$(CC) -o $< $(OBJS) $(LIBOPENCM3) $(LFLAGS) $(LFLAGS2OPT) $(CFLAGS)

	$(CP) -O binary $(TARGET).elf $(TARGET).bin
	
############################################
#生成DFU
$(TARGET).dfu: $(TARGET).bin
	../utils/dfu.py -v "$(DFUVERSION)" --name "$(HGVERSION) Firmware" $(DFU_ARGS):$< $@
	../utils/get_mem_usage.pl $(TARGET).map
