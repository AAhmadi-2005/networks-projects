CC = gcc

CFLAGS = -Wall -Wextra

all: dllsniffer dllsniffer2 dllsniffer_pro

dllsniffer: dllsniffer.c
	$(CC) $(CFLAGS) dllsniffer.c -o dllsniffer

dllsniffer2: dllsniffer2.c
	$(CC) $(CFLAGS) dllsniffer2.c -o dllsniffer2

dllsniffer_pro: dllsniffer_pro.c
	$(CC) $(CFLAGS) dllsniffer_pro.c -o dllsniffer_pro

clean:
	rm -f dllsniffer dllsniffer2 dllsniffer_pro