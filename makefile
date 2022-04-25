TARGET = bin/actinon

CC      = gcc
CFLAGS  = -Wall -O3 -std=c11 
LDFLAGS = -lbeth -lm -lpthread -latomic

MAIN_SRC = src
BETH_LIB = ../beth/out/libbeth.a

LIB_FOLDERS = \
	-L ../beth/out

INCLUDES = \
	-I ../beth/lib/bcore \
	-I ../beth/lib/bmath \
	-I ../beth/lib/bclos \
	-I ../beth/lib/badapt \
	-I ../beth/lib/bhvm \
	-I ../beth/lib/bhpt

SRCS = \
	$(wildcard $(MAIN_SRC)/*.c)

.PHONY: clean
.PHONY: run

$(TARGET): $(SRCS) $(BETH_LIB)
	mkdir -p $(dir $(TARGET) )
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(SRCS) $(LIB_FOLDERS) $(LDFLAGS)

$(BETH_LIB):
	make -C ../beth

run: $(TARGET) $(SRCS) $(BETH_LIB)
	$(TARGET)

clean:
	rm $(TARGET)

