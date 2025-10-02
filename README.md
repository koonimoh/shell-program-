# Custom C Shell

A small command-line shell written in C. It acts like a mini Unix shell: you can run programs, chain them with pipes, redirect input/output, and send work to the background.

## Features

* **Run programs** using `fork()` + `exec*()`.
* **Built-ins** like `cd` and `exit` run inside the shell process.
* **Pipes** with `|` (e.g., `ls | grep .c`).
* **Redirection** with `<`, `>`, `>>`.
* **Background jobs** with `&`.
* **Prompt** shows user/host and current directory.
* **Simple history** (recent commands).

## Build

```bash
make        # builds into bin/shell
make clean  # removes build output
```

## Run

```bash
./bin/shell
```

Examples:

```bash
ls -l
cat input.txt | grep error > errors.txt
sort < nums.txt >> all-sorted.txt
sleep 5 &
```

---

## How it works (internals)

Here’s the life of a command in this shell, step by step:

1. **Prompt**
   The shell prints a prompt built from your username, hostname, and the current directory (see `prompt.c`).

2. **Read a line**
   It reads what you type. Empty lines are ignored. Recent commands go into a small in-memory history.

3. **Lexing (tokenizing)**
   The input is split into tokens like words, `|`, `<`, `>`, and `&`. Quotes are handled so things like `"a b"` stay together (see `lexer.c`).

4. **Expansion**
   Basic expansions run here—like `~` for your home and `$VAR` for environment variables (see `expansion.c`).

5. **Parsing**
   Tokens are turned into a command structure: command + args, redirections, whether it’s background, and any pipeline stages (see `parser.c` and `include/shell.h`).

6. **Built-in check**
   If the first command is a built-in (e.g., `cd`, `exit`), the shell handles it directly in the parent process (see `builtins.c`). This matters because `cd` must change the current directory of the shell itself.

7. **Execution (no pipes)**
   For a single external command:

   * The shell sets up redirections with `open()` and `dup2()` (see `redirection.c`).
   * It `fork()`s.
   * In the child: do redirection, then `execvp()` the program.
   * In the parent: if foreground, `waitpid()` for it; if background, record the job and return to the prompt.

8. **Execution (with pipes)**
   For `cmd1 | cmd2 | cmd3` (see `piping.c`):

   * Create a pipe for each connection.
   * `fork()` each stage.
   * Wire up stdin/stdout with `dup2()` to the correct pipe ends.
   * `execvp()` each stage.
   * Close all pipe fds in the parent.
   * Wait for the last foreground pipeline (or track as a background job if `&` was used).

9. **Path search**
   If a command doesn’t contain `/`, the shell searches through the directories in `$PATH` to find the executable (see `path_search.c`).

10. **Cleanup & errors**
    The shell closes any leftover file descriptors, frees temporary memory, and prints a friendly error if something fails (file not found, permission denied, bad redirection, etc.).

---

## Project layout

```
include/       headers (shared types and function decls)
src/
  main.c       main loop (prompt → read → run)
  lexer.c      split input into tokens
  parser.c     build command structures
  expansion.c  ~ and $VAR expansions
  builtins.c   cd, exit, etc.
  execute.c    run external commands and jobs
  piping.c     connect multiple commands with pipes
  redirection.c handle <, >, >>
  path_search.c look up executables in $PATH
makefile       build rules
```

## Known limits

* Job control is basic (background start; no full `fg/bg` juggling).
* Globbing (`*.c`) is not expanded by the shell itself.
* History is simple (no interactive search/editing).
* Error messages are clear but minimal.

## Why this exists

This code is for learning. It shows how real shells work under the hood: parsing, forking, execing, pipes, redirection, and a dose of error handling. It’s small enough to read in an afternoon and hack on the next day.

---

