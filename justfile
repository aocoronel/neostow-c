# List (default)
list:
  just --list

# Build (debug)
build:
  cmake -B build .
  cmake --build build

# Build (release)
release:
  cmake -DCMAKE_BUILD_TYPE=Release -B build .
  cmake --build build

# Clean files
clean:
  rm -rf build

# Generate test sample to run forg << test-clean
test: test-clean
  cp -r test/ testing

# Removes test sample
test-clean:
  rm -rf testing/
