/**
 * @file efft.hpp
 * @brief Event-based Fast Fourier transform
 * @author Raul Tapia (raultapia.com)
 * @copyright GNU General Public License v3.0
 * @see https://github.com/raultapia/efft
 * @note This is a header-only library
 */
#ifndef EFFT_HPP
#define EFFT_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <eigen3/Eigen/Core>
#include <ostream>
#include <set>
#include <stdint.h>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef EFFT_USE_FFTW3
#include <fftw3.h>
#endif

class Stimulus {
public:
  unsigned int row{0};
  unsigned int col{0};
  bool state{true};
  Stimulus() = default;
  Stimulus(const unsigned int row, const unsigned int col) : row{row}, col{col} {};
  Stimulus(const unsigned int row, const unsigned int col, const bool state) : row{row}, col{col}, state{state} {};
  bool operator==(const Stimulus &p) const { return (row == p.row && col == p.col); }
  bool operator!=(const Stimulus &p) const { return (row != p.row || col != p.col); }
  Stimulus &on() {
    state = true;
    return *this;
  }
  Stimulus &off() {
    state = false;
    return *this;
  }
  Stimulus &set(const bool s) {
    state = s;
    return *this;
  }
  Stimulus &toggle() {
    state = !state;
    return *this;
  }
  friend std::ostream &operator<<(std::ostream &os, const Stimulus &stimulus) {
    os << "Stimulus(row: " << stimulus.row << ", col: " << stimulus.col << ", state: " << (stimulus.state ? "on" : "off") << ")";
    return os;
  }
};

class Stimuli : public std::vector<Stimulus> {
  using std::vector<Stimulus>::vector;

  static inline uint64_t pack(int row, int col) {
    return (static_cast<uint64_t>(static_cast<uint32_t>(row)) << 32) | static_cast<uint32_t>(col);
  }

public:
  void on() {
    std::for_each(begin(), end(), [](Stimulus &stimulus) { stimulus.state = true; });
  }
  void off() {
    std::for_each(begin(), end(), [](Stimulus &stimulus) { stimulus.state = false; });
  }
  void set(const bool state) {
    std::for_each(begin(), end(), [state](Stimulus &stimulus) { stimulus.state = state; });
  }
  void toggle() {
    std::for_each(begin(), end(), [](Stimulus &stimulus) { stimulus.state = !stimulus.state; });
  }

  /**
   * @brief Filters out stimuli.
   * @note This method is provided only for convenience.
   */
  void filter() {
    std::vector<Stimulus> out;
    std::unordered_map<uint64_t, size_t> pos;
    out.reserve(size());
    pos.reserve(size() * 2);

    for(const auto &s : *this) {
      uint64_t key = pack(s.row, s.col);
      auto [it, inserted] = pos.emplace(key, out.size());
      if(inserted) {
        out.emplace_back(s);
      } else {
        auto &chosen = out[it->second];
        if(s.state && !chosen.state) {
          chosen = s; // prefer state true over false
        }
      }
    }
    this->assign(out.begin(), out.end());
  }
};

using cfloat = std::complex<float>;
using cfloatmat = Eigen::Matrix<cfloat, Eigen::Dynamic, Eigen::Dynamic>;

constexpr unsigned int LOG2(const unsigned int n) { return ((n < 2) ? 0 : 1 + LOG2(n >> 1U)); }

#ifdef __has_builtin
#if __has_builtin(__builtin_ffs)
#define EFFT_HAS_BUILTIN_FFS
#endif
#endif
#ifdef EFFT_HAS_BUILTIN_FFS
inline unsigned int log2i(const unsigned int n) { return __builtin_ffs(static_cast<int>(n)) - 1; }
#else
inline unsigned int log2i(const unsigned int n) { return static_cast<unsigned int>(std::log2f(n)); }
#endif

template <unsigned int N>
class eFFT {
private:
  static constexpr unsigned int LOG2_N = LOG2(N);
  std::array<std::vector<cfloatmat>, LOG2_N + 1> tree_;
  std::array<cfloat, static_cast<std::size_t>(N) * static_cast<std::size_t>(N + 1)> twiddle_;
#ifdef EFFT_USE_FFTW3
  std::array<fftw_complex, static_cast<std::size_t>(N) * static_cast<std::size_t>(N)> fftwInput_;
  std::array<fftw_complex, static_cast<std::size_t>(N) * static_cast<std::size_t>(N)> fftwOutput_;
  fftw_plan plan_{nullptr};
#endif

public:
  eFFT() {
    constexpr float MINUS_TWO_PI = -2 * M_PI;
    for(unsigned int i = 0; i < N; i++) {
      for(unsigned int n = 0; n <= N; n++) {
        twiddle_[i + N * n] = static_cast<cfloat>(std::polar(1.0F, MINUS_TWO_PI * static_cast<float>(i) / static_cast<float>(n)));
      }
    }
  }

  ~eFFT() {
#ifdef EFFT_USE_FFTW3
    if(plan_ != nullptr) {
      fftw_destroy_plan(plan_);
    }
#endif
  }

  eFFT(const eFFT &) = default;
  eFFT(eFFT &&) noexcept = default;
  eFFT &operator=(const eFFT &) = default;
  eFFT &operator=(eFFT &&) noexcept = default;

  /**
   * @brief Get the frame size of the FFT.
   * @return The frame size as an unsigned integer.
   */
  [[nodiscard]] constexpr unsigned int framesize() const {
    return N;
  }

  /**
   * @brief Initializes the FFT computation with zero matrix.
   */
  void initialize() {
    cfloatmat f0(cfloatmat::Zero(N, N));
    initialize(f0);
  }

  /**
   * @brief Initializes the FFT computation with the provided matrix.
   *
   * @param x Input matrix.
   * @param offset Offset for the tree structure.
   */
  void initialize(cfloatmat &x, const int offset = 0) {
    const unsigned int n = x.rows();
    if(n == 1) {
      tree_[0].emplace_back(x);
      return;
    }
    const unsigned int ndiv2 = n >> 1U;
    const unsigned int idx = log2i(ndiv2);

    cfloatmat s00(x(Eigen::seq(Eigen::fix<0>, Eigen::last, Eigen::fix<2>), Eigen::seq(Eigen::fix<0>, Eigen::last, Eigen::fix<2>)));
    cfloatmat s01(x(Eigen::seq(Eigen::fix<0>, Eigen::last, Eigen::fix<2>), Eigen::seq(Eigen::fix<1>, Eigen::last, Eigen::fix<2>)));
    cfloatmat s10(x(Eigen::seq(Eigen::fix<1>, Eigen::last, Eigen::fix<2>), Eigen::seq(Eigen::fix<0>, Eigen::last, Eigen::fix<2>)));
    cfloatmat s11(x(Eigen::seq(Eigen::fix<1>, Eigen::last, Eigen::fix<2>), Eigen::seq(Eigen::fix<1>, Eigen::last, Eigen::fix<2>)));
    initialize(s00, 4 * offset);
    initialize(s01, 4 * offset + 4);
    initialize(s10, 4 * offset + 8);
    initialize(s11, 4 * offset + 12);

    const cfloatmat &x00 = tree_[idx][offset];
    const cfloatmat &x01 = tree_[idx][offset + 1];
    const cfloatmat &x10 = tree_[idx][offset + 2];
    const cfloatmat &x11 = tree_[idx][offset + 3];
    const unsigned int Nn = N * n;
    for(unsigned int i = 0; i < ndiv2; i++) {
      for(unsigned int j = 0; j < ndiv2; j++) {
        const cfloat tu = twiddle_[j + Nn] * x01(i, j);
        const cfloat td = twiddle_[(i + j) + Nn] * x11(i, j);
        const cfloat ts = twiddle_[i + Nn] * x10(i, j);

        const cfloat a = x00(i, j) + tu;
        const cfloat b = x00(i, j) - tu;
        const cfloat c = ts + td;
        const cfloat d = ts - td;

        x(i, j) = a + c;
        x(i, j + ndiv2) = b + d;
        x(i + ndiv2, j) = a - c;
        x(i + ndiv2, j + ndiv2) = b - d;
      }
    }
    tree_[log2i(n)].emplace_back(x);
  }

  /**
   * @brief Updates the FFT with a single stimulus.
   *
   * @param p The stimulus to update.
   * @return True if the update changed the FFT state, false otherwise.
   */
  bool update(const Stimulus &p) { return update(tree_[LOG2_N][0], p); }

  /**
   * @brief Updates the FFT with a single stimulus in the specified matrix.
   *
   * @param x The matrix to update.
   * @param p The stimulus to update.
   * @return True if the update changed the FFT state, false otherwise.
   */
  bool update(cfloatmat &x, const Stimulus &p, const unsigned int offset = 0) {
    const unsigned int n = x.rows();
    if(n == 1) {
      return std::exchange(x(0, 0), p.state).real() != static_cast<float>(p.state);
    }
    const unsigned int ndiv2 = n >> 1U;
    const unsigned int nndiv2 = n * ndiv2;
    const unsigned int idx = log2i(ndiv2);

    bool changed = false;
    if(static_cast<bool>(p.row & 1U)) {
      if(static_cast<bool>(p.col & 1U)) {
        changed = update(tree_[idx][offset + 3], {p.row >> 1U, p.col >> 1U, p.state}, 4 * offset + 12); // odd-odd
      } else {
        changed = update(tree_[idx][offset + 2], {p.row >> 1U, p.col >> 1U, p.state}, 4 * offset + 8); // odd-even
      }
    } else {
      if(static_cast<bool>(p.col & 1U)) {
        changed = update(tree_[idx][offset + 1], {p.row >> 1U, p.col >> 1U, p.state}, 4 * offset + 4); // even-odd
      } else {
        changed = update(tree_[idx][offset], {p.row >> 1U, p.col >> 1U, p.state}, 4 * offset); // even-even
      }
    }

    if(changed) {
      const cfloat *x00 = tree_[idx][offset].data();
      const cfloat *x01 = tree_[idx][offset + 1].data();
      const cfloat *x10 = tree_[idx][offset + 2].data();
      const cfloat *x11 = tree_[idx][offset + 3].data();
      cfloat *xp = x.data();
      const unsigned int Nn = N * n;

      for(unsigned int j = 0; j < ndiv2; j++) {
        const unsigned int ndiv2j = ndiv2 * j, nj = n * j;
        for(unsigned int i = 0; i < ndiv2; i++) {
          const unsigned int k = i + ndiv2j, k1 = i + nj, k2 = k1 + ndiv2;

          const cfloat tu = twiddle_[j + Nn] * x01[k];
          const cfloat td = twiddle_[i + j + Nn] * x11[k];
          const cfloat ts = twiddle_[i + Nn] * x10[k];

          const cfloat x00_k = x00[k];
          const cfloat a = x00_k + tu;
          const cfloat b = x00_k - tu;
          const cfloat c = ts + td;
          const cfloat d = ts - td;

          xp[k1] = a + c;
          xp[k1 + nndiv2] = b + d;
          xp[k2] = a - c;
          xp[k2 + nndiv2] = b - d;
        }
      }
    }
    return changed;
  }

  /**
   * @brief Updates the FFT with multiple stimuli.
   *
   * @param pv The stimuli to update.
   * @return True if the update changed the FFT state, false otherwise.
   */
  bool update(Stimuli &pv) { return update(tree_[LOG2_N][0], pv.begin(), pv.end()); }

  /**
   * @brief Updates the FFT with multiple stimuli in the specified matrix.
   *
   * @param x The matrix to update.
   * @param b0 Iterator pointing to the begining of the stimuli vector.
   * @param e0 Iterator pointing to the end of the stimuli vector.
   * @return True if the update changed the FFT state, false otherwise.
   */
  bool update(cfloatmat &x, Stimuli::iterator b0, Stimuli::iterator e0, const unsigned int offset = 0) {
    const unsigned int n = x.rows();
    if(n == 1) {
      if(e0 - b0 == 1) {
        return std::exchange(x(0, 0), b0->state).real() != static_cast<float>(b0->state);
      }
      const bool state = std::any_of(b0, e0, [](const Stimulus &p) { return p.state; });
      return std::exchange(x(0, 0), state).real() != static_cast<float>(state);
    }
    const unsigned int ndiv2 = n >> 1U;
    const unsigned int nndiv2 = n * ndiv2;
    const unsigned int idx = log2i(ndiv2);

    Stimuli::iterator e1, e2, e3;
    e2 = std::partition(b0, e0, [](const Stimulus &p) { return p.row & 1U; });
    e1 = std::partition(b0, e2, [](const Stimulus &p) { return p.col & 1U; });
    e3 = std::partition(e2, e0, [](const Stimulus &p) { return p.col & 1U; });

    auto transformStimuli = [](Stimuli::iterator begin, Stimuli::iterator end) {
      for(auto it = begin; it != end; ++it) {
        it->row >>= 1U;
        it->col >>= 1U;
      }
    };
    transformStimuli(b0, e1);
    transformStimuli(e1, e2);
    transformStimuli(e2, e3);
    transformStimuli(e3, e0);

    bool changed = false;
    if(b0 != e1) {
      changed = update(tree_[idx][offset + 3], b0, e1, 4 * offset + 12) || changed; // odd-odd
    }
    if(e1 != e2) {
      changed = update(tree_[idx][offset + 2], e1, e2, 4 * offset + 8) || changed; // odd-even
    }
    if(e2 != e3) {
      changed = update(tree_[idx][offset + 1], e2, e3, 4 * offset + 4) || changed; // even-odd
    }
    if(e3 != e0) {
      changed = update(tree_[idx][offset], e3, e0, 4 * offset) || changed; // even-even
    }

    if(changed) {
      const cfloat *x00 = tree_[idx][offset].data();
      const cfloat *x01 = tree_[idx][offset + 1].data();
      const cfloat *x10 = tree_[idx][offset + 2].data();
      const cfloat *x11 = tree_[idx][offset + 3].data();
      cfloat *xp = x.data();
      const unsigned int Nn = N * n;

      for(unsigned int j = 0; j < ndiv2; j++) {
        const unsigned int ndiv2j = ndiv2 * j, nj = n * j;
        for(unsigned int i = 0; i < ndiv2; i++) {
          const unsigned int k = i + ndiv2j, k1 = i + nj, k2 = k1 + ndiv2;

          const cfloat tu = twiddle_[j + Nn] * x01[k];
          const cfloat td = twiddle_[i + j + Nn] * x11[k];
          const cfloat ts = twiddle_[i + Nn] * x10[k];

          const cfloat x00_k = x00[k];
          const cfloat a = x00_k + tu;
          const cfloat b = x00_k - tu;
          const cfloat c = ts + td;
          const cfloat d = ts - td;

          xp[k1] = a + c;
          xp[k1 + nndiv2] = b + d;
          xp[k2] = a - c;
          xp[k2 + nndiv2] = b - d;
        }
      }
    }
    return changed;
  }

#ifdef EFFT_USE_FFTW3
  /**
   * @brief Initialize the FFT ground truth (FFTW) using the given image.
   *
   * @param image A complex float matrix to initialize the FFT input. Defaults to a zero matrix.
   */
  void initializeGroundTruth(const cfloatmat &image = cfloatmat::Zero(N, N)) {
    plan_ = fftw_plan_dft_2d(N, N, fftwInput_.data(), fftwOutput_.data(), FFTW_FORWARD, FFTW_ESTIMATE | FFTW_UNALIGNED | FFTW_NO_SIMD | FFTW_PRESERVE_INPUT);
    for(Eigen::Index i = 0; i < image.rows(); i++) {
      for(Eigen::Index j = 0; j < image.cols(); j++) {
        fftwInput_[N * i + j][0] = image(i, j).real();
        fftwInput_[N * i + j][1] = image(i, j).imag();
      }
    }
    fftw_execute(plan_);
  }

  /**
   * @brief Update the FFT ground truth (FFTW) with a single stimulus.
   *
   * @param p The stimulus to update.
   */
  void updateGroundTruth(const Stimulus &p) {
    fftwInput_[N * p.row + p.col][0] = static_cast<double>(p.state);
    fftwInput_[N * p.row + p.col][1] = 0;
    fftw_execute(plan_);
  }

  /**
   * @brief Update the FFT ground truth (FFTW) with multiple stimuli.
   *
   * @param pv The stimuli to update.
   */
  void updateGroundTruth(const Stimuli &pv) {
    std::set<std::pair<unsigned int, unsigned int>> activated;
    for(const Stimulus &p : pv) {
      if(p.state) {
        activated.insert({p.row, p.col});
      } else if(activated.find({p.row, p.col}) != activated.end()) {
        continue;
      }
      fftwInput_[N * p.row + p.col][0] = static_cast<double>(p.state);
      fftwInput_[N * p.row + p.col][1] = 0;
    }
    fftw_execute(plan_);
  }
#endif

  /**
   * @brief Get the FFT result as an Eigen matrix of complex floats.
   *
   * @return The FFT result.
   */
  [[nodiscard]] inline Eigen::Matrix<cfloat, N, N> getFFT() const {
    return tree_[LOG2_N][0];
  }

#ifdef EFFT_USE_FFTW3
  /**
   * @brief Get the FFT ground truth (FFTW) result as an Eigen matrix of complex floats.
   *
   * @return The ground truth FFT result.
   */
  [[nodiscard]] inline Eigen::Matrix<cfloat, N, N> getGroundTruthFFT() const {
    return Eigen::Map<const Eigen::Matrix<std::complex<double>, N, N, Eigen::RowMajor>>(reinterpret_cast<const std::complex<double> *>(fftwOutput_.data())).template cast<cfloat>();
  }

  /**
   * @brief Check the difference between the computed FFT and the ground truth FFT (FFTW).
   *
   * @return The norm of the difference between the computed FFT and the ground truth FFT.
   */
  [[nodiscard]] inline double check() const {
    return (getFFT() - getGroundTruthFFT()).norm();
  }
#endif
};

#endif // EFFT_HPP
