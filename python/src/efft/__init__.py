from ._efft import Stimulus, Stimuli
from ._efft import eFFT4, eFFT8, eFFT16, eFFT32, eFFT64, eFFT128, eFFT256, eFFT512, eFFT1024


def eFFT(n):
    mapping = {
        4: eFFT4,
        8: eFFT8,
        16: eFFT16,
        32: eFFT32,
        64: eFFT64,
        128: eFFT128,
        256: eFFT256,
        512: eFFT512,
        1024: eFFT1024,
    }
    try:
        return mapping[n]()
    except KeyError:
        raise ValueError(f"Unsupported FFT size: {n}")


__all__ = ["Stimulus", "Stimuli", "eFFT4", "eFFT8", "eFFT16", "eFFT32", "eFFT64", "eFFT128", "eFFT256", "eFFT512", "eFFT1024"]
