FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt -y update
RUN apt -y install build-essential
RUN apt -y install cmake
RUN apt -y install git
RUN apt -y install libeigen3-dev
RUN apt -y install libfftw3-dev
RUN apt -y install libbenchmark-dev
RUN apt -y install libgtest-dev
RUN apt -y install python3
RUN apt -y install python3-pip

COPY . /efft
RUN rm -rf /efft/build
RUN mkdir /efft/build
WORKDIR /efft/build
RUN cmake ..
RUN make -j$(nproc)
RUN make test
RUN make benchmark

WORKDIR /efft/python
RUN rm -rf /efft/python/build
RUN mkdir /efft/python/build
RUN ["python3", "-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel"]
RUN ["cmake", "-S", ".", "-B", "build"]
RUN ["cmake", "--build", "build"]
RUN ["cmake", "--install", "build"]
RUN ["python3", "-m", "pip", "install", ".[test]"]
RUN ["python3", "-m", "pytest", "--verbose"]
