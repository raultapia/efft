#include "efft.hpp"
#include <gtest/gtest.h>
#include <random>
#include <sstream>
#include <string>

constexpr unsigned int NTEST = 25;

template <unsigned int N>
class RandEventGenerator {
public:
  RandEventGenerator() : gen(std::random_device{}()), dis(0, N - 1) {}
  Stimulus next() {
    return {dis(gen) % N, dis(gen) % N, dis(gen) % 2 == 0};
  }
  Stimulus next(const bool state) {
    return {dis(gen) % N, dis(gen) % N, state};
  }
  Stimuli next(unsigned int n) {
    Stimuli ret;
    while(static_cast<bool>(n--)) {
      ret.emplace_back(next());
    }
    return ret;
  }
  Stimuli next(unsigned int n, const bool state) {
    Stimuli ret;
    while(static_cast<bool>(n--)) {
      ret.emplace_back(next(state));
    }
    return ret;
  }

private:
  std::mt19937 gen;
  std::uniform_int_distribution<> dis;
};

TEST(StimulusTest, Equality) {
  const Stimulus s1(123, 456, false);
  const Stimulus s2(123, 456, false);
  const Stimulus s3(123, 456, true);
  const Stimulus s4(123, 654, false);

  ASSERT_EQ(s1, s2);
  ASSERT_EQ(s1, s3);
  ASSERT_NE(s1, s4);
}

TEST(StimulusTest, OutputStream) {
  const Stimulus s1(123, 456, true);
  const Stimulus s2(789, 101, false);

  std::ostringstream oss1;
  oss1 << s1;
  ASSERT_EQ(oss1.str(), std::string("Stimulus(row: 123, col: 456, state: on)"));

  std::ostringstream oss2;
  oss2 << s2;
  ASSERT_EQ(oss2.str(), std::string("Stimulus(row: 789, col: 101, state: off)"));
}

TEST(StimuliTest, Filter) {
  Stimuli ss;
  ss.emplace_back(23, 45);
  ss.emplace_back(23, 45);
  ss.emplace_back(14, 45);
  ss.emplace_back(23, 33);
  ss.emplace_back(231, 451, true);
  ss.emplace_back(231, 451, false);
  ss.emplace_back(141, 451, true);
  ss.emplace_back(231, 331, false);

  ASSERT_EQ(ss.size(), 4 + 4);
  ss.filter();
  ASSERT_EQ(ss.size(), 3 + 3);
}

TEST(StimuliTest, State) {
  Stimuli ss;
  ss.emplace_back(231, 451, true);
  ss.emplace_back(231, 451, false);
  ss.emplace_back(141, 451, true);
  ss.emplace_back(231, 331, false);

  ss.set(true);
  for(const Stimulus s : ss) {
    ASSERT_EQ(s.state, true);
  }

  ss.set(false);
  for(const Stimulus s : ss) {
    ASSERT_EQ(s.state, false);
  }
}

#ifdef EFFT_USE_FFTW3
class eFFTTest : public ::testing::TestWithParam<unsigned int> {
};

template <unsigned int FRAME_SIZE>
static void FeedWithEvents() {
  eFFT<FRAME_SIZE> efft;
  RandEventGenerator<FRAME_SIZE> rand;

  Stimulus s;
  for(unsigned int test = 0; test < NTEST; test++) {
    if(!test) {
      efft.initialize();
      efft.initializeGroundTruth();
    } else {
      efft.update(s);
      efft.updateGroundTruth(s);
    }
    ASSERT_LT(efft.check(), 0.001);
    s = rand.next();
  }
}
TEST(eFFTTest, FeedWithEvents) {
  FeedWithEvents<4>();
  FeedWithEvents<8>();
  FeedWithEvents<16>();
  FeedWithEvents<32>();
  FeedWithEvents<64>();
  FeedWithEvents<128>();
  FeedWithEvents<256>();
}

template <unsigned int FRAME_SIZE>
static void FeedWithTheSameEvent() {
  eFFT<FRAME_SIZE> efft;
  RandEventGenerator<FRAME_SIZE> rand;
  const Stimulus s = rand.next(true);

  for(unsigned int test = 0; test < NTEST; test++) {
    if(!test) {
      efft.initialize();
      efft.initializeGroundTruth();
    } else {
      ASSERT_EQ(efft.update(s), test == 1);
      efft.updateGroundTruth(s);
    }
    ASSERT_LT(efft.check(), 0.001);
  }
}
TEST(eFFTTest, FeedWithTheSameEvent) {
  FeedWithTheSameEvent<4>();
  FeedWithTheSameEvent<8>();
  FeedWithTheSameEvent<16>();
  FeedWithTheSameEvent<32>();
  FeedWithTheSameEvent<64>();
  FeedWithTheSameEvent<128>();
  FeedWithTheSameEvent<256>();
}

template <unsigned int FRAME_SIZE>
static void FeedWithPackets(const unsigned int PACKET_SIZE) {
  eFFT<FRAME_SIZE> efft;
  RandEventGenerator<FRAME_SIZE> rand;

  Stimuli ss;
  for(unsigned int test = 0; test < NTEST; test++) {
    if(!test) {
      efft.initializeGroundTruth();
      efft.initialize();
    } else {
      efft.updateGroundTruth(ss);
      efft.update(ss);
    }

    ASSERT_LT(efft.check(), 0.1);
    ss = rand.next(PACKET_SIZE);
  }
}
TEST_P(eFFTTest, FeedWithPackets) {
  const unsigned int p = GetParam();
  FeedWithPackets<4>(p);
  FeedWithPackets<8>(p);
  FeedWithPackets<16>(p);
  FeedWithPackets<32>(p);
  FeedWithPackets<64>(p);
  FeedWithPackets<128>(p);
  FeedWithPackets<256>(p);
}

template <unsigned int FRAME_SIZE>
static void FeedWithTheSamePacket(const unsigned int PACKET_SIZE) {
  eFFT<FRAME_SIZE> efft;
  RandEventGenerator<FRAME_SIZE> rand;
  const Stimuli ss = rand.next(PACKET_SIZE, true);

  for(unsigned int test = 0; test < NTEST; test++) {
    Stimuli aux;
    aux.assign(ss.begin(), ss.end());
    if(!test) {
      efft.initializeGroundTruth();
      efft.initialize();
    } else {
      efft.updateGroundTruth(aux);
      ASSERT_EQ(efft.update(aux), test == 1);
    }

    ASSERT_LT(efft.check(), 0.1);
  }
}
TEST_P(eFFTTest, FeedWithTheSamePacket) {
  const unsigned int p = GetParam();
  FeedWithTheSamePacket<4>(p);
  FeedWithTheSamePacket<8>(p);
  FeedWithTheSamePacket<16>(p);
  FeedWithTheSamePacket<32>(p);
  FeedWithTheSamePacket<64>(p);
  FeedWithTheSamePacket<128>(p);
  FeedWithTheSamePacket<256>(p);
}

INSTANTIATE_TEST_CASE_P(eFFTWithPackets, eFFTTest, ::testing::Values(1, 10, 100, 1000, 10000));
#endif
