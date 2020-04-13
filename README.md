# gaml

[![Build Status](https://travis-ci.org/HerveFrezza-Buet/gaml.svg?branch=master)](https://travis-ci.org/HerveFrezza-Buet/gaml)

A C++ generic programming library for machine learning, developped by <a href="https://github.com/P-Fred">Frédéric Pennerath</a>, <a href="https://github.com/jeremyfix">Jérémy Fix</a> and <a href="https://github.com/HerveFrezza-Buet">Hervé Frezza-Buet</a>.

# Documentation

The directory contains several source packages, the documentation for each of them can be found in the doc directory as a doxygen generated html file.

For example, the documentation of gaml-libsvm is accessible from doc/gaml-libsvm/index.html

# C++17

The compilation of gaml requires C++-17, which means you need gcc >= 7. On ubuntu 16.04, the latest installed gcc is version 5, therefore you need to do the following :
```
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-7 g++-7
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 100 --slave /usr/bin/g++ g++ /usr/bin/g++-5
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 150 --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --config gcc
```

You may need to adapt the update-alternatives install depending on the versions that are installed on your system.

# Unix Installation

First, get the files.

``` 
git clone https://github.com/HerveFrezza-Buet/gaml
``` 

Then, you can install all packages as follows. The commands below concern the installation of the gaml package, the installation of the other packages is similar. 
For a 32bit architecture: 

```
mkdir -p gaml/build
cd gaml/build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
sudo make install
cd ../..
```

For a Fedora-64bit architecture:

```
mkdir -p gaml/build
cd gaml/build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DLIB_SUFFIX=64
sudo make install
cd ../..
```

The default installation uses GNU compilers gcc/g++. However clang is also supported.
Just set CC and CXX environment variables as follows before running cmake.
```
export CC= /usr/bin/clang
export CXX= /usr/bin/clang++
```

The available packages are :
<dl>
<dt>gaml</dt> <dd>The core library. It provide generic tools for usual data handling in machine learning.</dd>
<dt>gaml-datasets</dt><dd>Various standard datasets in the gaml framework</dd>
<dt>gaml-linear</dt> <dd>Linear learning (LASSO and LARS) with gaml.</dd>
<dt>gaml-libsvm</dt> <dd><a href="http://www.csie.ntu.edu.tw/~cjlin/libsvm">Libsvm</a> support. libsvm should be installed first.</dd>
<dt>gaml-xtree</dt> <dd>Extreme decision trees support.</dd>
<dt>gaml-mlp</dt> <dd>Multi-layer perceptron support. <a href="https://github.com/jeremyfix/easykf">easykf</a> should be installed first.</dd>
</dl>

# Related projects


<dl>
<dt><a href="http://www.csie.ntu.edu.tw/~cjlin/libsvm">Libsvm</a></dt> <dd>SVM algorithms.</dd>
<dt><a href="https://github.com/jeremyfix/easykf">easykf</a></dt> <dd>C++ kalman filtering</dd>
<dt><a href="https://github.com/HerveFrezza-Buet/vq3">vq3</a></dt> <dd>C++ generic vector quantization</dd>
<dt><a href="https://github.com/HerveFrezza-Buet/rllib">RLlib</a></dt> <dd>C++ generic reinforcement learning</dd>
</dl>
