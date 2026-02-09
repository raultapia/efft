#include "efft.hpp"
#include <complex>
#include <nanobind/eigen/dense.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/bind_vector.h>
#include <nanobind/stl/string.h>
#include <string>
#include <utility>

namespace nb = nanobind;
using namespace nb::literals;

static void bind_stimulus(nb::module_ &m) {
  nb::class_<Stimulus>(m, "Stimulus")
      .def(nb::init<>())
      .def(nb::init<unsigned int, unsigned int>(), "row"_a, "col"_a)
      .def(nb::init<unsigned int, unsigned int, bool>(), "row"_a, "col"_a, "state"_a)
      .def_rw("row", &Stimulus::row)
      .def_rw("col", &Stimulus::col)
      .def_rw("state", &Stimulus::state)
      .def("on", &Stimulus::on, nb::rv_policy::reference_internal)
      .def("off", &Stimulus::off, nb::rv_policy::reference_internal)
      .def("set", &Stimulus::set, "state"_a, nb::rv_policy::reference_internal)
      .def("toggle", &Stimulus::toggle, nb::rv_policy::reference_internal)
      .def("__repr__", [](const Stimulus &s) { return "<Stimulus(row=" + std::to_string(s.row) + ", col=" + std::to_string(s.col) + ", state=" + (s.state ? "on" : "off") + ")>"; });
}

static void bind_stimuli(nb::module_ &m) {
  nb::bind_vector<std::vector<Stimulus>>(m, "StimulusVector");
  nb::class_<Stimuli, std::vector<Stimulus>>(m, "Stimuli")
      .def(nb::init<>())
      .def("on", &Stimuli::on)
      .def("off", &Stimuli::off)
      .def("set", &Stimuli::set, "state"_a)
      .def("toggle", &Stimuli::toggle);
}

template <unsigned int N>
struct Bindings {
  eFFT<N> eng;
  void initialize() { eng.initialize(); }
  bool update(const Stimulus &stimulus) { return eng.update(stimulus); }
  bool update(Stimuli &stimuli) { return eng.update(stimuli); }
  Eigen::Matrix<std::complex<float>, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> get_fft() const { return eng.getFFT(); }
  int framesize() const { return static_cast<int>(eng.framesize()); }
};

template <unsigned int N>
static void bind_efft(nb::module_ &m, const std::string &pyname) {
  nb::class_<Bindings<N>>(m, pyname.c_str())
      .def(nb::init<>())
      .def("initialize", &Bindings<N>::initialize)
      .def("update", nb::overload_cast<const Stimulus &>(&Bindings<N>::update), "stimulus"_a)
      .def("update", nb::overload_cast<Stimuli &>(&Bindings<N>::update), "stimuli"_a)
      .def("get_fft", &Bindings<N>::get_fft)
      .def_prop_ro("framesize", &Bindings<N>::framesize);
}

NB_MODULE(_efft, m) {
  bind_stimulus(m);
  bind_stimuli(m);
  bind_efft<4>(m, "eFFT4");
  bind_efft<8>(m, "eFFT8");
  bind_efft<16>(m, "eFFT16");
  bind_efft<32>(m, "eFFT32");
  bind_efft<64>(m, "eFFT64");
  bind_efft<128>(m, "eFFT128");
  bind_efft<256>(m, "eFFT256");
  bind_efft<512>(m, "eFFT512");
  bind_efft<1024>(m, "eFFT1024");
}
