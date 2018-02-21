CC      = gcc
CFLAGS  = -Wall -O3 -std=c11 
LDFLAGS = -lm -lpthread

BETH_SRCS    = ../../beth-master/lib
ACTINON_SRCS = ../src

BETH_BCORE_SRCS = $(BETH_SRCS)/bcore
BETH_BCLOS_SRCS = $(BETH_SRCS)/bclos

INCLUDES = \
    -I $(BETH_BCORE_SRCS) \
    -I $(BETH_BCLOS_SRCS)

SRCS = \
    $(wildcard $(BETH_BCORE_SRCS)/*.c) \
    $(wildcard $(BETH_BCLOS_SRCS)/*.c) \
    $(wildcard $(ACTINON_SRCS)/*.c)
    
HDRS = \
    $(wildcard $(BETH_BCORE_SRCS)/*.h) \
    $(wildcard $(BETH_BCLOS_SRCS)/*.h) \
    $(wildcard $(ACTINON_SRCS)/*.h)

TARGET = actinon

$(TARGET): $(SRCS) $(HDRS)
	$(CC) -o $@ $(SRCS) $(CFLAGS) $(INCLUDES) $(LDFLAGS) 

.PHONY: clean

clean:
	rm -f $(TARGET)
