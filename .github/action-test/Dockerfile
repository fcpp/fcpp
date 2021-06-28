FROM ubuntu:latest
MAINTAINER giorgio.audrito@gmail.com

# Install needed libraries
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update &&\
    apt-get -qq -y install doxygen texlive texlive-font-utils cmake googletest gcc-10 g++-10 &&\
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 60 --slave /usr/bin/g++ g++ /usr/bin/g++-10 &&\
    apt-get -qq clean all

COPY "entrypoint.sh" "/entrypoint.sh"
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
