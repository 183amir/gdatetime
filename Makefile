all: gdatetime-tests

WARNINGS = -Wall -Werror -Wold-style-definition -Wdeclaration-after-statement \
	   -Wredundant-decls -Wmissing-noreturn -Wshadow -Wcast-align \
	   -Wwrite-strings -Winline -Wformat-nonliteral -Wformat-security \
	   -Wswitch-enum -Wswitch-default -Winit-self -Wmissing-include-dirs \
	   -Wundef -Waggregate-return -Wmissing-format-attribute \
	   -Wnested-externs

gdatetime-tests: gdatetime.c gdatetime.h gdatetime-tests.c
	gcc -g -o $@ $(WARNINGS) gdatetime.c gdatetime-tests.c `pkg-config --libs --cflags glib-2.0`

clean:
	rm -rf gdatetime-tests
