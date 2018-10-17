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
RUN git clone https://github.com/jeremyfix/popot.git; cd popot; mkdir build; cd build; cmake .. -DCMAKE_INSTALL_PREFIX=/usr; sudo make install

# Compile and test
RUN git clone https://github.com/jeremyfix/neuralfield.git; cd neuralfield; mkdir build; cd build; cmake .. -DCMAKE_INSTALL_PREFIX=/usr; sudo make install
