from unittest import TestCase
import numpy as np
from nes_py.nes_env import NESEnv
from nes_py.tests.rom_file_abs_path import rom_file_abs_path

RIGHT = 0b10000000
START = 0b00001000
A = 0b00000001

class ShouldKeepStatusBarStatic(TestCase):
    def test(self):
        env = NESEnv(rom_file_abs_path('super-mario-bros-3.nes'))
        env.reset()
        # Skip the initial screen
        for _ in range(60):
            env.step(0)
        # Press start twice to reach the world map
        for _ in range(5):
            env.step(START)
        for _ in range(10):
            env.step(0)
        for _ in range(5):
            env.step(START)
        for _ in range(20):
            env.step(0)
        # Enter the first level
        env.step(A)
        for _ in range(60):
            state, _, _, _ = env.step(0)
        before = state[224:].copy()
        # Move right for a while
        for _ in range(120):
            state, _, _, _ = env.step(RIGHT)
        after = state[224:]
        env.close()
        self.assertTrue(np.array_equal(before, after))
