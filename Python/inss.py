import os
import sys
import math


NSEC_PER_SEC = 1000000000.0
USEC_PER_SEC = 1000000.0

def RAD_TO_DEG(r):
    return (r / math.pi) * 180.0
class IMUFrame():
    def __init__(self, x, y, z, wx, wy, wz):
        self.x = x
        self.y = y
        self.z = z
        self.wx = wx
        self.wy = wy
        self.wz = wz

    @staticmethod
    def from_line(line):
        parts = line.split('\t')
        if parts[0].startswith('[TEST]'):
            # test script input reformat:
            longfmt = line.split()
            parts = [
                longfmt[4],
                longfmt[6],
                longfmt[8],
                longfmt[10],
                longfmt[12],
                longfmt[14],
            ]
        return IMUFrame(
            float(parts[0]),
            float(parts[1]),
            float(parts[2]),
            float(parts[3]),
            float(parts[4]),
            float(parts[5])
        )

class AHRS():
    pitch = 0.0
    roll = 0.0
    yaw = 0.0

    def integrate(self, frame, dt):
        self.pitch += frame.wx * (dt / USEC_PER_SEC)
        self.roll += frame.wy * (dt / USEC_PER_SEC)
        self.yaw += frame.wz * (dt / USEC_PER_SEC)

    def reset(self):
        self.pitch = 0.0
        self.roll = 0.0
        self.yaw = 0.0

    def __str__(self):
        return str({
            "pitch": str(RAD_TO_DEG(self.pitch)) + ' deg',
            "roll": str(RAD_TO_DEG(self.roll)) + ' deg',
            "yaw": str(RAD_TO_DEG(self.yaw)) + ' deg'
        })


def run(f):
    ahrs = AHRS()
    
    for line in f:
        frame = IMUFrame.from_line(line)
        print(frame.__dict__)
        ahrs.integrate(frame, USEC_PER_SEC)
        print(ahrs)


def main():
    
    if os.isatty(0):
        with open('sample.tsv') as f:
            run(f)
    else:
        print("Reading from stdin")
        for i in range(100):
            sys.stdin.readline()

        run(sys.stdin)

main()