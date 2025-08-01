from unittest import TestCase
import numpy as np
from nes_py.nes_env import NESEnv
from nes_py.tests.rom_file_abs_path import rom_file_abs_path
from nes_py.wrappers import JoypadSpace

RIGHT = JoypadSpace._button_map['right']
LEFT = JoypadSpace._button_map['left']
START = JoypadSpace._button_map['start']
A = JoypadSpace._button_map['A']

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
        # Move back and forth for a while to exercise IRQ timing
        for i in range(120):
            if i % 2:
                state, _, _, _ = env.step(RIGHT)
            else:
                state, _, _, _ = env.step(LEFT)
        after = state[224:]
        env.close()
        self.assertTrue(np.array_equal(before, after))


class ShouldAcknowledgeIRQ(TestCase):
    def test(self):
        env = NESEnv(rom_file_abs_path('super-mario-bros-3.nes'))
        env.reset()
        for _ in range(60):
            env.step(0)
        for _ in range(5):
            env.step(START)
        for _ in range(10):
            env.step(0)
        for _ in range(5):
            env.step(START)
        for _ in range(20):
            env.step(0)
        env.step(A)
        for _ in range(60):
            state, _, _, _ = env.step(0)
        baseline = state[224:].copy()
        for i in range(10):
            env.step(RIGHT if i % 2 else LEFT)
        mid = env.screen[224:].copy()
        self.assertTrue(np.array_equal(baseline, mid))
        env.bus_write(0xE000, 0)
        env.bus_write(0xE001, 0)
        for i in range(10):
            env.step(RIGHT if i % 2 else LEFT)
        final = env.screen[224:]
        env.close()
        self.assertTrue(np.array_equal(baseline, final))
