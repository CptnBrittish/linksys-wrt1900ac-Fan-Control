SFLAGS:=-std=c99
LFLAGS:=-lm -pthread -lc
WFLAGS:=-Wall -Werror -pedantic
BINARY:=fan_monitor

build: *.c
	$(CC) $(SFLAGS) $(LFLAGS) $(WFLAGS) -o $(BINARY) $+

clean:
	rm -f $(BINARY)
