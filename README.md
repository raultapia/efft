<div align="center" style="margin-bottom: 10px;">
<a href="https://github.com/raultapia/efft">
<img width="500" src="https://github.com/raultapia/efft/blob/main/.github/assets/efft.png?raw=true" alt="efft">
</a>
</div>
<p align="justify" class="brief">
We introduce eFFT, an efficient method for the calculation of the exact Fourier transform of an asynchronous event stream. It is based on keeping the matrices involved in the Radix-2 FFT algorithm in a tree data structure and updating them with the new events, extensively reusing computations, and avoiding unnecessary calculations while preserving exactness. eFFT can operate event-by-event, requiring for each event only a partial recalculation of the tree since most of the stored data are reused. It can also operate with event packets, using the tree structure to detect and avoid unnecessary and repeated calculations when integrating the different events within each packet to further reduce the number of operations.
</p>

## ⚙️ Installation

eFFT is provided as a header-only file for easy integration and relies solely on C++ standard and [Eigen3](https://eigen.tuxfamily.org/) libraries.

> **Note:**
> [FFTW3](https://fftw.org/) served as the benchmark for testing and evaluation. To enable it, define `EFFT_USE_FFTW3` during compilation (e.g., `-DEFFT_USE_FFTW3`).

## 📦 Dependencies

For C++ usage, the following dependencies are required:
* C++ compiler with C++17
* CMake ≥ 3.20.0
* Eigen ≥ 3.4.0
* FFTW3 ≥ 3.3.8 (optional; see `EFFT_USE_FFTW3`)
* (dev-only) GTest, Google Benchmark

For Python usage, dependencies are defined in `python/pyproject.toml` and the build dependencies are fetchable via `python/CMakeLists.txt`.

## 🖥️ Usage

Here's a minimal working example:

```cpp
eFFT<128> efft;           // Instance
efft.initialize();        // Initialization

Stimulus e(1, 1, true);   // Event insertion
efft.update(e);           // Insert event

efft.getFFT();            // Get result as Eigen matrix
```

And another example handling event packets:

```cpp
eFFT<128> efft;                   // Instance
efft.initialize();                // Initialization

Stimuli events;
events.emplace_back(1, 1, true);  // Insert event
events.emplace_back(2, 2, true);  // Insert event
events.emplace_back(3, 3, false); // Extract event
efft.update(events);              // Insert event

efft.getFFT();                    // Get result as Eigen matrix
```

Please refer to the [official documentation](https://raultapia.github.io/efft/) for more details.

## 🐍 Python Bindings

The eFFT library also provides Python bindings for seamless integration into Python-based workflows. These bindings are built using [nanobind](https://github.com/wjakob/nanobind) and offer the same functionality as the C++ library. You can build and install the bindings using the following commands:

```bash
cd python
pip install .
```

However, you can also use PyPI to install the package directly:

```bash
pip install efft
```

Here's an example of how to use the Python bindings:

```python
from efft import Stimulus, Stimuli, eFFT

efft = eFFT(128)                  # Create an eFFT instance with a frame size of 128
efft.initialize()

event = Stimulus(1, 1, True)      # Insert a single event
efft.update(event)

fft_result = efft.get_fft()       # Retrieve the FFT result

events = Stimuli()                # Insert multiple events
events.append(Stimulus(2, 2, True))
events.append(Stimulus(3, 3, False))
efft.update(events)

fft_result = efft.get_fft()       # Retrieve the updated FFT result
```

## 📜 Citation

If you use this work in an academic context, please cite the following publication:

> R. Tapia, J.R. Martínez-de Dios, A. Ollero
> **eFFT: An Event-based Method for the Efficient Computation of Exact Fourier Transforms**,
> IEEE Transactions on Pattern Analysis and Machine Intelligence, 2024.

```bibtex
@article{tapia2024efft,
  author={Tapia, R. and Martínez-de Dios, J.R. and Ollero, A.},
  journal={IEEE Transactions on Pattern Analysis and Machine Intelligence},
  title={{eFFT}: An Event-based Method for the Efficient Computation of Exact {Fourier} Transforms},
  year={2024},
  volume={46},
  number={12},
  pages={9630-9647},
  doi={10.1109/TPAMI.2024.3422209}
}
```

## 📝 License

Distributed under the GPLv3 License. See [`LICENSE`](https://github.com/raultapia/efft/tree/main/LICENSE) for more information.

## 📬 Contact

[Raul Tapia](https://raultapia.com) - raultapia@us.es
