#include "efft.hpp"
#include <benchmark/benchmark.h>
#include <random>

constexpr unsigned int NTEST = 250;

template <unsigned int N>
class RandEventGenerator {
public:
  RandEventGenerator() : gen(std::random_device{}()), dis(0, N - 1) {}
  Stimulus next() {
    return {dis(gen) % N, dis(gen) % N, dis(gen) % 2 == 0};
  }
  Stimuli next(unsigned int n) {
    Stimuli ret;
    while(static_cast<bool>(n--)) {
      ret.emplace_back(next());
    }
    return ret;
  }

private:
  std::mt19937 gen;
  std::uniform_int_distribution<> dis;
};

template <unsigned int FRAME_SIZE>
static void BenchmarkFeedWithEventsFFTW(benchmark::State &state) {
  eFFT<FRAME_SIZE> efft;
  efft.initializeGroundTruth();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimulus s;
  for(auto _ : state) {
    for(unsigned int test = 0; test < NTEST; test++) {
      s = rand.next();
      efft.updateGroundTruth(s);
      [[maybe_unused]] auto result = efft.getGroundTruthFFT();
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithEventsFFTW, 16);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEventsFFTW, 32);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEventsFFTW, 64);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEventsFFTW, 128);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEventsFFTW, 256);

template <unsigned int FRAME_SIZE>
static void BenchmarkFeedWithPacketsFFTW(benchmark::State &state) {
  eFFT<FRAME_SIZE> efft;
  efft.initializeGroundTruth();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimuli ss;
  for(auto _ : state) {
    for(unsigned int test = 0; test < NTEST; test++) {
      ss = rand.next(state.range(0));
      efft.updateGroundTruth(ss);
      [[maybe_unused]] auto result = efft.getGroundTruthFFT();
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithPacketsFFTW, 128)->Arg(1000)->Arg(5000)->Arg(10000)->Arg(50000)->Arg(100000)->Arg(500000)->Iterations(10);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPacketsFFTW, 256)->Arg(1000)->Arg(5000)->Arg(10000)->Arg(50000)->Arg(100000)->Arg(500000)->Iterations(10);

template <unsigned int FRAME_SIZE>
static void BenchmarkFeedWithEvents(benchmark::State &state) {
  eFFT<FRAME_SIZE> efft;
  efft.initialize();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimulus s;
  for(auto _ : state) {
    for(unsigned int test = 0; test < NTEST; test++) {
      s = rand.next();
      efft.update(s);
      [[maybe_unused]] auto result = efft.getFFT();
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 16);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 32);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 64);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 128);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 256);

template <unsigned int FRAME_SIZE>
static void BenchmarkFeedWithPackets(benchmark::State &state) {
  eFFT<FRAME_SIZE> efft;
  efft.initialize();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimuli ss;
  for(auto _ : state) {
    for(unsigned int test = 0; test < NTEST; test++) {
      ss = rand.next(state.range(0));
      efft.update(ss);
      [[maybe_unused]] auto result = efft.getFFT();
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 128)->Arg(1000)->Arg(5000)->Arg(10000)->Arg(50000)->Arg(100000)->Arg(500000)->Iterations(10);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 256)->Arg(1000)->Arg(5000)->Arg(10000)->Arg(50000)->Arg(100000)->Arg(500000)->Iterations(10);

BENCHMARK_MAIN();
