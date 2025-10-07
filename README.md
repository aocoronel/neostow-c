# The declarative GNU Stow

`neostow` is a tool that streamline the process to manage symlinks like GNU stow, but using a configuration file. It allows for more flexible symlink management, enabling the creation of symlinks from anywhere to anywhere on your computer.

This declarative nature allows to easily make reproducible and granular symlinking, unlike GNU Stow. However, this project does not aims to fully replace GNU Stow, but to give a declarative feature missing from it.

You can also look for a version of this script written in Bash at [aocoronel/neostow-sh](https://github.com/aocoronel/neostow-sh), or a legacy version written in Nim at [aocoronel/neostow-nim](https://github.com/aocoronel/neostow-nim).

## Features

- **Flexible Symlink Creation**: Create symlinks from any source to any destination.
- **Per-Project Configuration**: Maintain a `.neostow` configuration file per project.
- **Overwrite Symlinks**: Optionally overwrite existing symlinks.
- **Remove Symlinks**: Easily remove all created symlinks.

## Installation

```bash
git clone https://github.com/aocoronel/neostow-c.git
cd neostow-c || return 1
cmake -DCMAKE_BUILD_TYPE=Release -B build .
cmake --install build
```

## Usage

`neostow` reads from a `.neostow` file in the current directory to determine which symlinks to create. The `.neostow` file should contain lines in the following format: `source=destination`.

See the manpage(1) at `FILES` for more details.

````console
neostow | The Declarative GNU Stow

Usage:  neostow [OPTIONS] <COMMAND>

Commands:
  autocomplete SHELL
          Generate autocompletion for bash or zsh
  delete
          Delete symlinks
  edit
          Edit the configuration file

Options:
  -D, --debug
          Enables debug verbosity
  -F, --force
          Skip prompt dialogs
  -V, --verbose
          Enable verbosity
  -c, --config <CONFIG>
          Load an alternative config
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

````

config/myconfig=/home/username/.config/myconfig/ scripts/myscript.sh=/home/username/bin/myscript/

````

## Integrations

### [Just](https://github.com/casey/just)

`just` is a handy way to save and run `neostow` commands from any directory within the project.

In or `justfile`, you may create a recipe like this:

```just
# Neostow: Verbose and overwrite
neostow:
  neostow -v -r
````

Then, from any child directory where this `justfile` was placed, you can just run `just neostow`, and it will run the configured recipe.

## License

This repository is licensed under the MIT License, a very permissive license that allows you to use, modify, copy, distribute and more.
