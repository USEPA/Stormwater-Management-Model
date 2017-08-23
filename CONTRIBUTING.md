# Setting up the development environment

We use the [conda](https://conda.io/docs/user-guide/getting-started.html) as
infrastructure to develop, build and test SWMM. We also use [CMake](https://cmake.org/)
to have a simple way to generate cross platform binaries.

Conda is a cross platform package manager written in Python that is part of the
scientific distribution [Anaconda](https://www.continuum.io/what-is-anaconda),
and also provides isolated system environments where one can install several
different packages.

## Step 1: Install conda via Anaconda or Miniconda

- Download and install [miniconda](https://conda.io/miniconda.html) or
[anaconda](https://www.continuum.io/downloads)

**Note**: We recommend installing for the current user and avoid `All users`
installations.

## Step 2: Install conda-build and anaconda-client

Open up a **terminal** (on OSX or Linux) or the **Anaconda Prompt**
(on Windows), and install **conda-build** and **anaconda-client** on the
**root** environment.

The **root** environment is the default conda environment that gets installed
with anaconda or miniconda. Conda-build is what allows us to create a conda
package with a recipe. The recipe for SWMM can be found in the
[conda.recipe](https://github.com/OpenWaterAnalytics/Stormwater-Management-Model/tree/develop/conda.recipe)
folder.

```
$ conda install conda-build anaconda-client
```

## Step 3: create the development enviornment

So now, lets create a conda environment called **swmm** and install the
development dependencies. We also install **clangdev** from the **conda-forge**
to use the [clang-format](https://clang.llvm.org/docs/ClangFormat.html) tool
to keep the C code tidy and uniform.

### On OSX and Linux:

```
$ conda create --name swmm cmake gcc libgcc python=3.6
$ conda install --name clangdev --channel conda-forge
```

### On Windows:

```
$ conda create --name swmm cmake python=3.6
$ conda install --name clangdev --channel conda-forge
```

## Step 4: Activating the conda environment

By default when opening a new Terminal (or Anaconda Prompt) we will get the
**root** environment so everytime we start working we need to make sure that
we do:

### On OSX and Linux:

```
$ source activate swmm
```

### On Windows:

```
$ activate swmm
```

Now we are ready to do some work!


# Contributing to the project

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


## Coding guidelines

Coding guidelines are simple, because we use an automatic tool to format the
code after making changes make sure to run the tool on the file that changed
by doing:

```
(swmm) $ python tools/clangformatter.py src/<file-that-was-edited> -i
```


# Building the project for local testing

If you want to build the package for local testing, open the terminal
(or Anaconda prompt) and type:

```
(swmm) $ conda-build conda.recipe
```

After the process has finished you can install and run the locally created
package with:

```
(swmm) $ conda install --name swmm libswmm --use-local
(swmm) $ run-swmm
```
