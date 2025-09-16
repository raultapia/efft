#include "efft.hpp"
#include <benchmark/benchmark.h>
#include <random>

constexpr unsigned int NTEST = 25;

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
static void BenchmarkFeedWithEvents(benchmark::State &state) {
  eFFT<FRAME_SIZE> efft;
  RandEventGenerator<FRAME_SIZE> rand;

  Stimulus s;
  for(auto _ : state) {
    efft.initialize();
    for(unsigned int test = 0; test < NTEST; test++) {
      efft.update(s);
      s = rand.next();
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 4);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 8);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 16);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 32);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 64);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 128);
BENCHMARK_TEMPLATE(BenchmarkFeedWithEvents, 256);

template <unsigned int FRAME_SIZE>
static void BenchmarkFeedWithPackets(benchmark::State &state) {
  eFFT<FRAME_SIZE> efft;
  RandEventGenerator<FRAME_SIZE> rand;

  Stimuli ss = rand.next(state.range(0));
  for(auto _ : state) {
    efft.initialize();
    for(unsigned int test = 0; test < NTEST; test++) {
      efft.update(ss);
    }
  }
}
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 4)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 8)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 16)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 32)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 64)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 128)->Arg(10)->Arg(100)->Arg(1000);
BENCHMARK_TEMPLATE(BenchmarkFeedWithPackets, 256)->Arg(10)->Arg(100)->Arg(1000);

BENCHMARK_MAIN();
