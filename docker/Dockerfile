# Ubuntu toolchain for building, testing and profiling investm, and for the
# open-source comparison (QuantLib, Google Benchmark; the Python libraries are
# installed from scripts/compare/requirements.txt).
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
      libquantlib0-dev libbenchmark-dev \
      python3 python3-pip python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/venv \
    && /opt/venv/bin/pip install --no-cache-dir numpy pytest pylint yapf cpplint
COPY scripts/compare/requirements.txt /tmp/compare-requirements.txt
RUN /opt/venv/bin/pip install --no-cache-dir -r /tmp/compare-requirements.txt
ENV PATH="/opt/venv/bin:${PATH}"

WORKDIR /develop/investm
CMD ["bash"]
