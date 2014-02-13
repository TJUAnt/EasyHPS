#set the compiler
CC := mpic++

#link pthread lib
LIBS := -lpthread

#EasyTHPS source files' path
VPATH += :EasyHPS_src/

#Target Directory
BIN := Bin/

#the target user application name
APP := EasyHPSAPP

THREAD = 6

SRCS := \
./SmithWaterman.cpp \
./Alignment.cpp \
./ScoringTable.cpp \
./FastaLib.cpp
