FROM ubuntu:latest
MAINTAINER giorgio.audrito@gmail.com

# Install needed libraries
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update &&\
    apt-get -qq -y install doxygen cmake googletest  &&\
    apt-get -qq clean all

COPY "entrypoint.sh" "/entrypoint.sh"
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
