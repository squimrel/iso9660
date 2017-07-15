# ISO 9660

A basic and incomplete implementation of ECMA-119.

## Build instructions

```
mkdir -p build/
cd build/
cmake -DCMAKE_INSTALL_PREFIX:PATH=$PWD ..
make
make install
```

## Development packaging

```
sh scripts/package.sh HEAD
```
