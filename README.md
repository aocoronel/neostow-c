# The declarative GNU stow

`neostow` is a tool that streamline the process to manage symlinks like GNU `stow`, but using a `neostow` file, instead. It allows more flexible symlink management, enabling the creation of symlinks from a relative source to anywhere on your computer.

This declarative nature allows to easily make reproducible and granular symlinking, unlike `stow`. However, this project does not aims to fully replace it, but to give a declarative feature missing from it.

This tool is useful to keep files and directories organized in a single centralized place, while also having them across the system. Differently, than `stow`, which allows you to place some files into a different target, other than the parent directory, `neostow` aims to further improve this functionality.

With `neostow` each file or directory can be symlinked to a specific part of the system, and not the project as a whole. There is not ignore file, and no need to adjust the folder layout to achieve your goals. If your `neostow` does not explicitly specify an operation, it won't touch a single file.

Other versions of Neostow:

- [aocoronel/neostow-sh](https://github.com/aocoronel/neostow-sh)
- [aocoronel/neostow-nim](https://github.com/aocoronel/neostow-nim): No longer maintained.
- [aocoronel/neostow-rs](https://github.com/aocoronel/neostow-rs)

## C Edition

This project is the third on the `neostow` rewrites series, mainly for C learning purposes. The original `neostow` is the POSIX shell version.

### Bugs

Currently, there is one known bug.

1. Using the `-f` or `--file` option does not work at all.

### Vulnerabilities

Because C is an easy language to introduce vulnerabilities, and I'm not an grey beard on C, I cannot review the code to give any guarantees, that there is no vulnerability in the code.

The C version runs `diff` to compare non-symbolic files to the source file. However, it does not use direct shell, which can be exploited with special constructs such as `||`, `&&` or `;`. This is a general improvement compared to the `neostow-sh`, but no guarantee is given.

## Features

- **Flexible symlink creation**: Create symlinks from any relative source to any destination.
- **Per-project file**: Maintain a `.neostow` file per project.
- **Overwrite symlinks**: Optionally overwrite existing symlinks.
- **Remove symlinks**: Easily remove all created symlinks.
- **Preview operations**: Preview what operations would run.

## Installation

Copy the source:

```bash
git clone https://github.com/aocoronel/neostow-c.git
cd neostow-c
```

To install `neostow`:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build .
cmake --install build
```

To install manpages:

```bash
sudo cp docs/neostow.1 /usr/share/man/man1/
```

To install completions:

```bash
# Bash
sudo neostow autocomplete bash > /usr/share/bash-completion/completions/_neostow
# Zsh
sudo neostow autocomplete zsh > /usr/share/zsh/site-functions/_neostow
```

## Usage

`neostow` reads from a `.neostow` file in the current directory to determine which symlinks to create.

The `.neostow` file should contain lines in the following format: `source=destination`.

See the manpage(1) at `FILES` for more details. Or give at look at the [example file](https://github.com/aocoronel/neostow-c/blob/main/test/.neostow) and [Examples](#examples).

```console
neostow | the declarative GNU stow

Usage:  neostow [OPTIONS] <COMMAND>

Commands:
  autocomplete SHELL
          Generate autocompletion for bash or zsh
  delete
          Delete symlinks
  edit
          Edit the neostow file

Options:
  -D, --debug
          Enables debug verbosity
  -F, --force
          Skip prompt dialogs
  -V, --verbose
          Enable verbosity
  -f, --file <FILE>
          Load an alternative neostow file
  -d, --dry
          Describe potential operations
  -h, --help
          Displays this message and exits
  -o, --overwrite
          Overwrite existing symlinks
  -v, --version
          Displays program version
```

### Configuration File

The `.neostow` file should be placed in the root of your project directory.

#### Examples

Example `.neostow` file:

```text
config/myconfig.txt=/home/username/.config/myconfig/ # links myconfig.txt to ~/.config/myconfig/
scripts/myscript.sh=/home/username/bin/myscript/ # links myscript.sh to ~/bin/myscrypt/
myfile=$HOME/Downloads # links myfile to ~/Downloads
```

The left side paths are relative to the current directory where the `.neostow` file is found.

## Integrations

### [Just](https://github.com/casey/just)

`just` is a handy way to save and run `neostow` commands from any directory within the project.

Because, `neostow` has the limitation to only find `.neostow` files in the current directory, `just` gives an extra functionality. Integrating both, allows you to run `neostow` from the root of the project, thus finding the `.neostow` file.

In or `justfile`, you may create a recipe like this:

```just
# Neostow: Overwrite
neostow:
  neostow -o
```

Then, from any child directory where this `justfile` was placed, you can just run `just neostow`, and it will run the configured recipe.

## Notes

This program was only tested in a Linux machine.

## License

This repository is licensed under the MIT License, a very permissive license that allows you to use, modify, copy, distribute and more.
