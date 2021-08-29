# MySHell-msh
A simple Linux shell for learning.


## How to Compile
- install git
- install CMake

In Ubuntu, you can install them with
```bash
$ sudo apt install git
$ sudo apt install cmake
```
Then, run these commands:
```bash
$ git clone git@github.com:Apple-Zhang/MySHell-msh.git
$ cd MySHell-msh
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## How to use
Go to `build` folder and run `msh` with
```bash
$ ./msh
```
You can run `help` or `?` command in `msh` to obtain help information.

## Notice
This project is made for assignment of OS lesson at Shenzhen University,
which implements part of very basic functions commonly used in `bash`.
It should be noted that the program might exists some potential bugs which have not been fixed properly.
