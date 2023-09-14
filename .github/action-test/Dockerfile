FROM debian:bookworm
LABEL maintainer="giorgio.audrito@gmail.com"

# Install needed libraries
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update &&\
    apt-get -qq -y --no-install-recommends install g++ make cmake ninja-build doxygen-latex python3 &&\
    apt-get -qq clean all

COPY "entrypoint.sh" "/entrypoint.sh"
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
