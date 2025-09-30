#!/usr/bin/env python3
from efft import Stimulus, Stimuli, eFFT
import numpy as np
import random


def test_stimulus():
    s1 = Stimulus()
    s2 = Stimulus(1, 2)
    s3 = Stimulus(1, 2, False)

    assert s1.row == 0
    assert s1.col == 0
    assert s1.state

    assert s2.row == 1
    assert s2.col == 2
    assert s2.state

    assert s3.row == 1
    assert s3.col == 2
    assert not s3.state

    s1.off()
    assert not s1.state

    s1.on()
    assert s1.state

    s1.set(False)
    assert not s1.state

    s1.toggle()
    assert s1.state

    assert repr(s3) == "<Stimulus(row=1, col=2, state=off)>"


def test_stimuli():
    stimuli = Stimuli()
    assert len(stimuli) == 0

    s = Stimulus(1, 2, False)
    stimuli.append(s)
    assert len(stimuli) == 1
    assert stimuli[0].row == 1
    assert stimuli[0].col == 2
    assert not stimuli[0].state

    stimuli.append(Stimulus(1, 2, False))
    stimuli.append(Stimulus(1, 2, False))
    stimuli.append(Stimulus(1, 2, False))
    assert len(stimuli) == 4

    stimuli.on()
    for s in stimuli:
        assert s.state
    stimuli.off()
    for s in stimuli:
        assert not s.state
    stimuli.set(True)
    for s in stimuli:
        assert s.state
    stimuli.toggle()
    for s in stimuli:
        assert not s.state


def test_efft():
    def generate_random_stimulus(framesize):
        row = random.randint(0, framesize - 1)
        col = random.randint(0, framesize - 1)
        state = random.choice([True, False])
        return Stimulus(row, col, state)

    def generate_random_stimuli(framesize, size):
        stimuli = Stimuli()
        for _ in range(size):
            stimuli.append(generate_random_stimulus(framesize))
        return stimuli

    for k in range(2, 10):
        efft = eFFT(2**k)

        assert efft.framesize == 2**k

        efft.initialize()
        gt = np.zeros((efft.framesize, efft.framesize))

        fft_result_prev = efft.get_fft()
        for _ in range(100):
            stimulus = generate_random_stimulus(2**k)
            gt[stimulus.row, stimulus.col] = 1 if stimulus.state else 0
            r = efft.update(stimulus)

            fft_result = efft.get_fft()
            assert not (r == np.array_equal(fft_result, fft_result_prev))
            assert fft_result.shape == (efft.framesize, efft.framesize)
            fft_result_prev = np.copy(fft_result)

            expected_fft = np.fft.fft2(gt)
            np.testing.assert_array_almost_equal(fft_result, expected_fft, decimal=3)

        fft_result_prev = efft.get_fft()
        for n in range(5, 100):
            stimuli = generate_random_stimuli(2**k, n)
            activated = set()
            for s in stimuli:
                h = hash((s.row, s.col))
                if s.state:
                    activated.add(h)
                elif h in activated:
                    continue
                gt[s.row, s.col] = 1 if s.state else 0
            r = efft.update(stimuli)

            fft_result = efft.get_fft()
            assert not (r == np.array_equal(fft_result, fft_result_prev))
            assert fft_result.shape == (efft.framesize, efft.framesize)
            fft_result_prev = np.copy(fft_result)

            expected_fft = np.fft.fft2(gt)
            np.testing.assert_array_almost_equal(fft_result, expected_fft, decimal=1)
