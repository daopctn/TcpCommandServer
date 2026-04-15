#!/usr/bin/env python3
"""Generate one CSV per message type with 1000 rows each."""

import random
import os

os.makedirs("data", exist_ok=True)

N = 1000

# track.csv
with open("data/track.csv", "w") as f:
    f.write("f1,f2,f3,f4a,f4b,f4c\n")
    for _ in range(N):
        f.write(f"{random.randint(0,65535)},{random.randint(0,255)},"
                f"{random.randint(0,65535)},{random.randint(0,255)},"
                f"{random.randint(0,255)},{random.randint(0,255)}\n")
print(f"data/track.csv  — {N} rows")

# track2.csv
with open("data/track2.csv", "w") as f:
    f.write("f1,f2,f3,f4a,f4b,f4c\n")
    for _ in range(N):
        f.write(f"{random.randint(0,65535)},{random.randint(0,255)},"
                f"{random.randint(0,65535)},{random.randint(0,255)},"
                f"{random.randint(0,255)},{random.randint(0,255)}\n")
print(f"data/track2.csv — {N} rows")

# angle.csv  — sequential values wrapping at 65535
with open("data/angle.csv", "w") as f:
    f.write("angle\n")
    for i in range(N):
        f.write(f"{i % 65536}\n")
print(f"data/angle.csv  — {N} rows")

# clutter.csv
with open("data/clutter.csv", "w") as f:
    f.write("value1,v2a,v2b,v2c,value3\n")
    for _ in range(N):
        f.write(f"{random.randint(0,65535)},{random.randint(0,255)},"
                f"{random.randint(0,255)},{random.randint(0,255)},"
                f"{random.randint(0,65535)}\n")
print(f"data/clutter.csv — {N} rows")

# video2.csv  — entries field: semicolon-separated 24-bit integers (no quotes needed,
# no commas inside so the field is unambiguous)
with open("data/video2.csv", "w") as f:
    f.write("header,entries\n")
    for _ in range(N):
        header = random.randint(0, 65535)
        count  = random.randint(1, 20)
        entries = ";".join(str(random.randint(0, 16777215)) for _ in range(count))
        f.write(f"{header},{entries}\n")
print(f"data/video2.csv  — {N} rows")
