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

> [!NOTE]
> [FFTW3](https://fftw.org/) served as the benchmark for testing and evaluation. To enable it, define `EFFT_USE_FFTW3` during compilation (e.g., `-DEFFT_USE_FFTW3`).


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
  volume={0},
  number={0},
  pages={0000-0000},
  doi={10.1109/TPAMI.2024.3422209}
}
```

## 📝 License

Distributed under the GPLv3 License. See [`LICENSE`](https://github.com/raultapia/efft/tree/main/LICENSE) for more information.

## 📬 Contact

[Raul Tapia](https://raultapia.com) - raultapia@us.es
