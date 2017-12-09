# I. Setting up the development environment (This is a work in progress)

The official windows compiler used for originally building SWMM was Visual Studio 2010 32 bits and as such is the recommended compiler on windows for current development. We are in the process of validating that other compilers on windows for different architecture and different operating systems results in the same results (Regression tests).

If you are also testing the PySWMM wrapper for development, we advised to use Python 3.4 (32 bits) which uses the same compiler and hence provides full binary compatibility for SWMM

## A.) Windows 

### Install Dependencies

- Visual Studio C++ 10. Link to download
- Install CMake >= `<VERSION>` Link to download
- Python 3.4. Link to download
- Install the nrtest dependencies. Links to download

### Building
To build SWMM then run:

```
$ cmake -G "Visual Studio <VERSION>" -DCMAKE_INSTALL_PREFIX:PATH=output -DCMAKE_BUILD_TYPE:STRING=Release
```

### Running

```
$ run-swmm --help
```

## B.) Linux

### Install Dependencies
- Build dependencies... gcc >=  libgcc >=
- cmake
- Install the nrtest dependencies..... Links to download

#### On Debian (.deb) based systems you can use
```
$ sudo apt install ...
```

#### On Centos (.rpm) based systems you can use

```
$ sudo yum install ...
```

### Building

```
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE:STRING=Release ..
$ make -j $CPU_COUNT
$ make install
```

### Running

```
$ run-swmm --help
```

## C.) OSX 

### InstallDependencies
- Clang version >= ??? or
- gcc version >= ???
- cmake
- Install python and the nrtest dependencies..... Links to download or commands

#### If using Homebrew
```
$ brew install ...
```

### Building

```
$ cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE:STRING=Release ..
$ make -j $CPU_COUNT
$ make install
```

### Running

```
$ run-swmm --help
```


# II. If you are developing for PySwmm

If you are developing for PySwmm you need to install these versions to work with [how python is compiled](https://wiki.python.org/moin/WindowsCompilers) on different platforms:

## A.) Windows

### Python 2.7
- Visual Studio C++ 9 for Python 2.7. Link to download
- Python 2.7. Link to download

### Python 3.4
- Visual Studio C++ 10 for Python 3.4. Link to download
- Python 3.4. Link to download.

### Python 3.5 and 3.6
- Visual Studio C++ 14 for Python 3.5 and 3.6. Link to download
- Python 3.5. Link to download
- Python 3.6. Link to download

## B.) Linux

....

## C.) OSX

....


# III. Regression Testing

To execute regression tests run:

```
$ python <script...>
```

# IV. Unit tests

To execute unit tests run:

```
$ python <script...>
```


# V. Contributing to the project

## Git and Github

We use git and github for all development and any time you want to make a
contribution make sure to follow these steps.

### Step 1: Fork the repository
Make sure to fork the `https://github.com/OpenWaterAnalytics/Stormwater-Management-Model`
repo so that you now have on your local account `https://github.com/<github-username>/Stormwater-Management-Model`

### Step 2: Add the original repo as remote

On the terminal type:
```
$ git remote add upstream https://github.com/OpenWaterAnalytics/Stormwater-Management-Model.git
```

### Step 3: Make a branch from develop, make changes and push code

Update local repository and create branch `fix/some-branch-fix`

```
$ git checkout develop
$ git pull upstream develop
$ git push origin develop
$ git checkout -b fix/some-branch-fix
```

Make changes and then commit them with sensible messages.

```
$ git add .
$ git commit -m "My awesome message!"
$ git push origin fix/some-branch-fix
```

### Step 4: Make Pull request

Once the work is done go to https://github.com/OpenWaterAnalytics/Stormwater-Management-Model
and make a pull request from your created branch.

