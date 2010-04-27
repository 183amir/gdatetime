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
	gcalendarjulian.c \
	$(NULL)

HEADERS = \
	gdatetime.h \
	gcalendar.h \
	gcalendargregorian.h \
	gcalendarjulian.h \
	$(NULL)

gdatetime-tests: $(FILES) $(HEADERS)
	gcc -g -o $@ $(WARNINGS) $(FILES) `pkg-config --libs --cflags gobject-2.0`

clean:
	rm -rf gdatetime-tests

valgrind: gdatetime-tests
	 G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --leak-check=full --leak-resolution=high --suppressions=gtk.suppression ./gdatetime-tests

test: gdatetime-tests
	./gdatetime-tests
