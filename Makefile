all: gdatetime-tests

WARNINGS = -Wall -Werror -Wold-style-definition -Wdeclaration-after-statement \
	   -Wredundant-decls -Wmissing-noreturn -Wshadow -Wcast-align \
	   -Wwrite-strings -Winline -Wformat-nonliteral -Wformat-security \
	   -Wswitch-enum -Wswitch-default -Winit-self -Wmissing-include-dirs \
	   -Wundef -Waggregate-return -Wmissing-format-attribute \
	   -Wnested-externs

FILES = \
	gdatetime.c \
	gdatetime-tests.c \
	gcalendar.c \
	gcalendargregorian.c \
	$(NULL)

HEADERS = \
	gdatetime.h \
	gcalendar.h \
	gcalendargregorian.h \
	$(NULL)

gdatetime-tests: $(FILES) $(HEADERS)
	gcc -g -o $@ $(WARNINGS) $(FILES) `pkg-config --libs --cflags gobject-2.0`

clean:
	rm -rf gdatetime-tests
