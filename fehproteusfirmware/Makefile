CC = arm-none-eabi-g++
LD = $(CC)

ifeq ($(OS),Windows_NT)
    SHELL := CMD
endif

SPECS = nosys.specs

INCLUDES = -I.. -I. -ILibraries/ -IDrivers/ -IStartup/ 
ARGS = -O0 -ffunction-sections -fdata-sections -fno-exceptions -c -fmessage-length=0 -Wno-psabi -specs=$(SPECS)
CFLAGS =  -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=cortex-m4 -mfloat-abi=soft -mthumb -g3 -gdwarf-2 -gstrict-dwarf

# This is a recursive implementation of the wildcard function provided by gnu.
# We use it to allow students to have an arbitrary source file structure, with multiple source files and folders
# https://stackoverflow.com/questions/2483182/recursive-wildcards-in-gnu-make/18258352#18258352
recursiveWildcard=$(foreach d,$(wildcard $(1:=/*)),$(call recursiveWildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Gets all of the header files (.h) in the parent directory and its children folders, 
# excluding the files we have in here, as those get built in the libraries target.
AUTOH := $(filter-out ../fehproteusfirmware/%, $(call recursiveWildcard, .., *.h))

# Gets all of the source files (.cpp) in the parent directory and its children folders, 
# excluding the files we have in here, as those get built in the libraries target.
AUTOCPP := $(filter-out ../fehproteusfirmware/%, $(call recursiveWildcard, .., *.cpp))

# Gets all of the source files (.cpp) in the parent directory and its children folders, 
# excluding the files we have in here, as those get built in the libraries target.
AUTOOBJECTS := $(patsubst %.cpp, %.o, $(AUTOCPP))

OBJECTS := $(AUTOOBJECTS) Startup/startup_mk60d10.o Libraries/FEHMotor.o Drivers/mcg.o Drivers/i2c.o Drivers/spi.o Drivers/uart.o Drivers/ff.o Drivers/SDHC.o Drivers/lptmr.o FEHProteus.o Drivers/FEHPropeller.o Libraries/FEHUtility.o Libraries/FEHIO.o Drivers/adc16.o Libraries/FEHBuzzer.o Libraries/FEHServo.o Libraries/FEHLCD.o Libraries/FEHAccel.o Libraries/FEHBattery.o Drivers/FEHXBee.o Libraries/FEHRCS.o Libraries/FEHSD.o Libraries/FEHRandom.o

ifeq ($(OS), Windows_NT)
DELOBJECTS := $(subst /,\, $(OBJECTS))
endif

all: $(TARGET).elf $(TARGET).s19

clean:
ifeq ($(OS),Windows_NT)
	del $(DELOBJECTS) ..\$(TARGET).elf ..\$(TARGET).s19 ..\$(TARGET).map $(DELOBJECTS:%.o=%.d)
else
	rm -f $(OBJECTS) ../$(TARGET).elf ../$(TARGET).s19 ../$(TARGET).map $(OBJECTS:%.o=%.d)
endif

%.o : %.c $(AUTOH)
	$(CC) $(INCLUDES) $(ARGS) $(CFLAGS) -c $< -o $@

%.o : %.cpp $(AUTOH)
	$(CC) $(INCLUDES) $(ARGS) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJECTS)
	@echo $(OBJECTS)
	$(LD) $(OBJECTS) -u _printf_float -u _scanf_float -TLinker/MK60DN512Z_flash.ld -Xlinker --gc-sections -Wl,-Map,../$(TARGET).map -n -specs=$(SPECS) -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -g3 -gdwarf-2 -gstrict-dwarf -g -o ../$(TARGET).elf

$(TARGET).s19: $(TARGET).elf
	arm-none-eabi-objcopy  -O srec --srec-len=40 --srec-forceS3 ../$(TARGET).elf ../$(TARGET).s19

deploy:
ifeq ($(OS),Windows_NT)
	@Tools\copyS19.bat
else
	@Tools/copyS19
endif

run:
	@arm-none-eabi-size  ../$(TARGET).elf > ../size.txt
	@echo ----------------------------------------------
ifeq ($(OS),Windows_NT)
	@Tools\sizeGet.bat
else
	@Tools/sizeGet
endif
