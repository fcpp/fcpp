FROM ubuntu:19.10
MAINTAINER giorgio.audrito@gmail.com

# Install needed libraries
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update &&\
    apt-get -qq -y install software-properties-common python-yaml pypy asymptote bc git nano-tiny emacs-nox less htop procps curl gnupg2 doxygen texlive-font-utils &&\
    apt-get -qq clean all
RUN curl https://bazel.build/bazel-release.pub.gpg | apt-key add -
RUN echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y &&\
    apt-get -qq --allow-unauthenticated update &&\
    apt-get -qq --allow-unauthenticated -y install default-jdk gcc-9 g++-9 bazel build-essential &&\
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9 &&\
    apt-get -qq clean all
