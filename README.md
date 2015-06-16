# gaml

A C++ generic programming library for machine learning, developped by <a href="https://github.com/P-Fred">Frédéric Pennerath</a>, <a href="https://github.com/jeremyfix">Jérémy Fix</a> and <a href="https://github.com/HerveFrezza-Buet">Hervé Frezza-Buet</a>.

# Documentation

The directory contains several source packages, the documentation for each of them can be found in the doc directory as a doxygen generated html file.

For example, the documentation of gaml-libsvm is accessible from doc/gaml-libsvm/index.html


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
<dt>gaml-linear</dt> <dd>Linear learning (LASSO and LARS) with gaml.</dd>
<dt>gaml-libsvm</dt> <dd><a href="http://www.csie.ntu.edu.tw/~cjlin/libsvm">Libsvm</a> support. libsvm should be installed first.</dd>
<dt>gaml-xtree</dt> <dd>Extreme decision trees support.</dd>
<dt>gaml-mlp</dt> <dd>Multi-layer perceptron support. <a href="https://github.com/jeremyfix/easykf">easykf</a> should be installed first.</dd>
<dt>gaml-kvq</dt> <dd>Kernelized vector quantization support, by linking with <a href="https://github.com/HerveFrezza-Buet/vq2">vq2</a>. vq2 should be installed first. Notes that the basics for kernelized vector quantization are available in the core gaml package (the span namespace).</dd>
</dl>

# Related projects


<dl>
<dt><a href="http://www.csie.ntu.edu.tw/~cjlin/libsvm">Libsvm</a></dt> <dd>SVM algorithms.</dd>
<dt><a href="https://github.com/jeremyfix/easykf">easykf</a></dt> <dd>C++ kalman filtering</dd>
<dt><a href="https://github.com/HerveFrezza-Buet/vq2">vq2</a></dt> <dd>C++ generic vector quantization</dd>
<dt><a href="https://github.com/HerveFrezza-Buet/vq2">RLlib</a></dt> <dd>C++ generic reinforcement learning</dd>
</dl>
