# SO-shell

This repository contains your own `soshell` implementation in `src/`, separate from the class sheets in `docs/` and the reference project in `example/FP05/`.

Build with:

```sh
make
```

Run with:

```sh
./soshell
```

Implemented features:

- command loop and parser
- builtins: `?`, `sair`, `obterinfo`, `PS1=...`, `quemsoueu`, `cd`, `socp`
- numeric builtins: `epsilon`, `calc`, `bits`, `displayBitOps`
- file and media builtins: `isjpeg`, `isgif`, `isValid`, `openfile`, `closefd`, `read`, `fileinfo`
- external commands with `fork()` and `execvp()`
- background execution with `&`
- redirections: `<`, `>`, `>>`, `2>`
- a single pipe with `|`

Type `?` inside `soshell` to list the supported shell functions.

Useful local test files already present in the repository:

- `src/geef.gif` for `isgif`
- `example/FP05/image.jpg` for `isjpeg`

For simplicity, special tokens should be separated by spaces.
Examples: `ls > out.txt`, `cat file | wc -l`, `sleep 5 &`.