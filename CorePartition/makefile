CC=gcc
src = $(wildcard *.c)
srcpp = $(wildcard *.cpp)
obj = $(src:.c=.o) $(srcpp:.cpp=.o)

TARGET = CorePartition
LDFLAGS = -lm -L /opt/local/lib -I /opt/local/include

$(TARGET): $(obj)
	$(CC) -c $@ $^ (LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) $(TARGET)
