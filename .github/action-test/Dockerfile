FROM debian:trixie-slim
LABEL maintainer="giorgio.audrito@gmail.com"

# Install needed libraries
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update &&\
    apt-get -qq -y --no-install-recommends install g++ clang make cmake ninja-build doxygen-latex python3 ca-certificates &&\
    rm -rf /var/lib/apt/lists/*

COPY "entrypoint.sh" "/entrypoint.sh"
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
