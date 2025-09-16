FROM ubuntu:22.04

RUN apt -y update
RUN apt -y install build-essential
RUN apt -y install cmake
RUN apt -y install git
RUN apt -y install libeigen3-dev
RUN apt -y install libfftw3-dev
RUN apt -y install libbenchmark-dev
RUN apt -y install libgtest-dev

COPY . /efft
RUN rm -rf /efft/build
RUN mkdir /efft/build
WORKDIR /efft/build
RUN cmake ..
RUN make -j$(nproc)
RUN make test
RUN make benchmark
