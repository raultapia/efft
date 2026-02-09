FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive

RUN apt -y update
RUN apt -y install build-essential
RUN apt -y install cmake
RUN apt -y install git
RUN apt -y install libeigen3-dev
RUN apt -y install libfftw3-dev
RUN apt -y install libgtest-dev
RUN apt -y install libbenchmark-dev
RUN apt -y install python3
RUN apt -y install python3-pip
RUN apt -y install python3-venv

COPY . /efft
RUN rm -rf /efft/build
RUN mkdir /efft/build
WORKDIR /efft/build
RUN cmake ..
RUN make -j$(nproc)
RUN make test
RUN make run-benchmark

WORKDIR /efft/python
RUN rm -rf /efft/python/build
RUN mkdir /efft/python/build
RUN python3 -m venv .venv
RUN . .venv/bin/activate && python3 -m pip install -U pip
RUN . .venv/bin/activate && python3 -m pip install -v .[test]
RUN . .venv/bin/activate && python3 -m pytest --verbose
