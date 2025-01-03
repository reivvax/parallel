import sys

times = {
    "5": "0.002",
    "10": "0.0025",
    "15": "0.0094",
    "20": "0.106",
    "25": "0.92",
    "30": "7.104",
    "32": "7.104",
    "34": "32.01"
}

if len(sys.argv) == 2 and sys.argv[1] in times:
    print(times[sys.argv[1]])
else:
    print(f'Usage: python3 reference_times.py <n>\nwhere n is an element of {{{", ".join(times.keys())}}}.')
    sys.exit(1)