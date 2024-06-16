# SNEK

Simple game of Snake in C using Curses, written between ca. 2018-01-01 and
2018-02-22.

## Building from source

The C file is known to build with GCC (MinGW) on a Windows machine.  
The BAT file should "just werk" for such a setup.  
A version of Curses (e.g. NCurses, PDCurses) is required.  
Building for and running on UNIX/Linux is untested.

> Note: For purposes of validating, that the program can still be built from
> source, I've tried to build the program from source with the repository's
> current content and my current installation of MinGW; however, I've been
> unable to install NCurses (the original implementation of Curses used by the
> program) on my current machine; hence, I've rewritten it slightly, to use
> PDCurses instead.
> This has for some reason introduced a visible bug in the title screen, which I
> cannot attest existed in the 2018 NCurses build.
> Needless to say, it's an area of further investigation.

## Running the program

The board/field/play area/what-have-you takes on the dimensions of the CLI when
started; hence, it is best enjoyed, when starting a CLI instance first, sizing
the window to taste, and then starting the program.

## Known bugs

- Small memory leak after first death
- Small window size leads to larger-than-expected memory allocation
- Holding down any key (bar the escape key) makes the snake go faster (which is
  not necessarily a bug)
- Clicking on the CLI changes snake behaviour to only moving when a key is
  pressed: this is apparently [a bug with MinGW mouse support](https://lists.gnu.org/archive/html/bug-ncurses/2013-12/msg00001.html)
- Title screen is botched
