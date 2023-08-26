FROM debian:bullseye
MAINTAINER giorgio.audrito@gmail.com

# Install needed libraries
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update &&\
    apt-get -qq -y install doxygen texlive texlive-font-utils cmake ninja-build gcc g++ python3 &&\
    apt-get -qq clean all

COPY "entrypoint.sh" "/entrypoint.sh"
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
