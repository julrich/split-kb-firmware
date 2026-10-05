import os, sys, glob, time

proto = sys.argv[1]          # bInterfaceProtocol of the interface to watch
dur = float(sys.argv[2]) if len(sys.argv) > 2 else 240.0


def node():
    for d in sorted(glob.glob('/sys/class/hidraw/hidraw*')):
        try:
            name = open(d + '/device/uevent').read()
            p = open(d + '/device/../bInterfaceProtocol').read().strip()
        except OSError:
            continue
        if 'Sofle' in name and p == proto:
            return '/dev/' + os.path.basename(d)
    return None


t0 = time.time()
cur = fd = None
while time.time() - t0 < dur:
    n = node()
    if n is None:
        if fd is not None:
            os.close(fd)
            fd = None
            print('%6.2f  --- device gone' % (time.time() - t0), flush=True)
        time.sleep(0.2)
        continue
    if n != cur or fd is None:
        if fd is not None:
            os.close(fd)
        try:
            fd = os.open(n, os.O_RDONLY)
        except OSError as e:
            fd = None
            print('%6.2f  --- open %s failed: %s' % (time.time() - t0, n, e), flush=True)
            time.sleep(0.2)
            continue
        cur = n
        print('%6.2f  --- opened %s' % (time.time() - t0, n), flush=True)
    try:
        d = os.read(fd, 64)
    except OSError as e:
        print('%6.2f  --- read failed: %s' % (time.time() - t0, e), flush=True)
        os.close(fd)
        fd = None
        continue
    print('%6.2f  %s' % (time.time() - t0, ' '.join('%02x' % b for b in d)), flush=True)
