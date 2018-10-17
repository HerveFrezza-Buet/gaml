FROM buildpack-deps:bionic

MAINTAINER jeremyfix

# See https://serverfault.com/questions/683605/docker-container-time-timezone-will-not-reflect-changes
ENV TZ=Europe/Paris
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Update aptitude with new repo
RUN apt update

RUN apt install -y software-properties-common 
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt install -y g++-7 gcc-7 libgsl-dev libgsl23 

# Deps for easykf
RUN apt install -y libboost-python-dev 

# Deps for gaml-datasets
RUN apt install -y libcurl4

# Install our standard dependencies
RUN apt install -y git cmake sudo

# Install our own dependencies
# For libsvm
RUN git clone https://github.com/cjlin1/libsvm.git
RUN cd libsvm; make lib; sudo mkdir /usr/include/libsvm; sudo cp svm.h /usr/include/libsvm/ ; sudo cp libsvm.so.2 /usr/lib/ ; sudo ln -s /usr/lib/libsvm.so.2 /usr/lib/libsvm.so 

# Dependency for gaml-mlp
RUN git clone https://github.com/jeremyfix/easykf.git
RUN cd easykf; mkdir -p build ; cd build ; cmake .. -DCMAKE_INSTALL_PREFIX=/usr ; sudo make install  

# GAML
RUN git clone https://github.com/HerveFrezza-Buet/gaml
RUN cd gaml; mkdir -p build; cd build ; cmake .. -DCMAKE_INSTALL_PREFIX=/usr ; sudo make install    
RUN cd gaml-datasets; mkdir -p build; cd build ; cmake .. -DCMAKE_INSTALL_PREFIX=/usr ; sudo make install  
RUN cd gaml-xtree; mkdir -p build; cd build ; cmake .. -DCMAKE_INSTALL_PREFIX=/usr ; sudo make install  
RUN cd gaml-mlp; mkdir -p build; cd build ; cmake .. -DCMAKE_INSTALL_PREFIX=/usr ; sudo make install  
RUN cd gaml-libsvm; mkdir -p build; cd build ; cmake .. -DCMAKE_INSTALL_PREFIX=/usr ; sudo make install  
RUN cd gaml-linear; mkdir -p build; cd build ; cmake .. -DCMAKE_INSTALL_PREFIX=/usr ; sudo make install  
