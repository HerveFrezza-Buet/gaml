# gaml

A C++ generic programming library for machine learning

# Documentation

The directory contains several source packages, the documentation for each of them can be found in the doc directory as a doxygen generated html file.

For example, the documentation of gaml-libsvm is accessible from doc/gaml-libsvm/index.html


# Installation

First, get the files.

``` 
git clone https://github.com/HerveFrezza-Buet/gaml
``` 

Then, you can install all packages as follows. The commands below concern the installation of the gaml package, the installation of the other packages is similar.

```
mkdir -p gaml/build
cd gaml/build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
sudo make install
cd ../..
```

The available packages are :
<dl>
<dt>gaml</dt> The core library. It provide generic tools for usual data handling in machine learning.
</dll>