# Ubuntu toolchain for building, testing and profiling investm.
#
#   docker build -t investm-dev -f docker/Dockerfile .
#   docker run --rm -it -v C:\develop:/develop -w /develop/investm investm-dev
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build git ca-certificates \
      libgtest-dev libgmock-dev \
      clang-format clang-tidy \
      valgrind kcachegrind-converters linux-tools-generic \
      python3 python3-pip python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/venv \
    && /opt/venv/bin/pip install --no-cache-dir numpy pytest pylint yapf cpplint
ENV PATH="/opt/venv/bin:${PATH}"

WORKDIR /develop/investm
CMD ["bash"]
