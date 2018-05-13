# Simple POSIX shell: yxsh

## Requirements

* UNIX system
* GNU Readline
* Curses (for GNU Readline)
* GNU Bison

## Build

### Downloading sourse codes

```bash
git clone https://github.com/kirpichik/yxsh.git
cd yxsh
```

### Preparing for build

**If you have globally installed** `GNU Readline` and
`GNU Bison` **this step can be skipped.**

1. Open Makefile
2. Edit BISON, INCLUDES and READLINE_LIB variables for your environment.

#### BISON variable:

GNU Bison compiler for parser generation.

#### INCLUDES variable:

Flags to external libraries for include GNU Readline. Example:

```make
INCLUDE=-I/usr/local/opt/readline/include
```

#### READLINE_LIB variable:

Flags to external libraries for linking GNU Readline.
Example for dynamic linking:

```make
READLINE_LIB=-lcurses -L/usr/local/opt/readline/lib -lreadline
```

Or for static linking:

```make
READLINE_LIB=-lcurses /usr/local/opt/readline/lib/libreadline.a
```

### Finishing build

```bash
make
```

## Usage

```bash
./yxsh
```

### Features

All the features described in the shell requirements, excluding loops,
conditional jumps and logical expressions.

