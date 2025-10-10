#include "efft.hpp"
#include <benchmark/benchmark.h>
#include <cstddef>
#include <random>

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
  constexpr std::size_t num_events_to_process = 250;
  eFFT<FRAME_SIZE> efft;
  efft.initializeGroundTruth();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimulus s;
  for(auto _ : state) {
    for(std::size_t it = 0; it < num_events_to_process; it++) {
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
  constexpr std::size_t num_events_to_process = 500000;
  const std::size_t num_iterations = num_events_to_process / state.range(0);
  eFFT<FRAME_SIZE> efft;
  efft.initializeGroundTruth();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimuli ss;
  for(auto _ : state) {
    for(std::size_t it = 0; it < num_iterations; it++) {
      ss = rand.next(state.range(0));
      efft.updateGroundTruth(ss);
      [[maybe_unused]] auto result = efft.getGroundTruthFFT();
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithPacketsFFTW, 128)->Arg(100)->Arg(500)->Arg(1000)->Arg(2500)->Arg(5000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPacketsFFTW, 256)->Arg(100)->Arg(500)->Arg(1000)->Arg(2500)->Arg(5000);

template <unsigned int FRAME_SIZE>
static void BenchmarkFeedWithEvents(benchmark::State &state) {
  constexpr std::size_t num_events_to_process = 250;
  eFFT<FRAME_SIZE> efft;
  efft.initialize();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimulus s;
  for(auto _ : state) {
    for(std::size_t it = 0; it < num_events_to_process; it++) {
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
  constexpr std::size_t num_events_to_process = 500000;
  const std::size_t num_iterations = num_events_to_process / state.range(0);
  eFFT<FRAME_SIZE> efft;
  efft.initialize();
  RandEventGenerator<FRAME_SIZE> rand;

  Stimuli ss;
  for(auto _ : state) {
    for(std::size_t it = 0; it < num_iterations; it++) {
      ss = rand.next(state.range(0));
      efft.update(ss);
      [[maybe_unused]] auto result = efft.getFFT();
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 128)->Arg(100)->Arg(500)->Arg(1000)->Arg(2500)->Arg(5000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 256)->Arg(100)->Arg(500)->Arg(1000)->Arg(2500)->Arg(5000);

BENCHMARK_MAIN();
