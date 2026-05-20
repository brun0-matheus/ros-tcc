from subprocess import Popen, PIPE, STDOUT
from collections import namedtuple
import concurrent.futures
import re 
import os 
from tqdm import tqdm

THREADS = int(os.getenv('NUM_THREADS', 2))
NUM_IT = int(os.getenv('NUM_IT', 1))
PARAMS = []

PROGRAM_CPP = './main'
PROGRAM_SAGE_REDUCED = ['sage', 'attack_reduced.sage']
PROGRAM_SAGE_BINARY = ['sage', 'attack_binary.sage']

for (idx, curve) in enumerate(['P224', 'P256', 'P384', 'P521', 'secp256k1']):
    if curve == 'P521':
        continue 

    for ndim in [0, 2, 3, 4, 5, 6, 7]:
        PARAMS.append((f'cpp_{curve}_{ndim}', (PROGRAM_CPP, idx, ndim), NUM_IT))
        if ndim > 0:
            args = PROGRAM_SAGE_REDUCED + [idx, ndim]
        else:
            args = PROGRAM_SAGE_BINARY + [idx, ndim]

        PARAMS.append((f'sage_{curve}_{ndim}', args, NUM_IT))


Stats = namedtuple('Stats', ['name', 'success', 'time', 'attempts', 'num_sessions'])

RE_TIME = re.compile(r'Took (\d+\.\d+) seconds.')
RE_SESS = re.compile(r'Number of sessions: (\d+)')
RE_ATTEMPT = re.compile(r'Number of attempts: (\d+)')

class Process:
    def __init__(self, cmdline):
        self.p = Popen(cmdline, stdin=PIPE, stdout=PIPE, stderr=STDOUT)

    def sendline(self, data: bytes):
        self.p.stdin.write(data + b'\n')
        self.p.stdin.flush()

    def recvline(self):
        return self.p.stdout.readline()

    def close(self):
        self.p.terminate()


def run(name, args):
    args = [str(x) for x in args]
    p = Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE)

    try:
        out, err = p.communicate()
        out = out.decode()
        err = err.decode()
    except TimeoutExpired:
        print('TIMEOUT on ', name, args)
        return None 


    t = RE_TIME.search(out)
    nsess = RE_SESS.search(out)

    if t is None or nsess is None:
        if t is None:
            print('Could not match the running time')
        else:
            print('Could not match the number of sessions')

        print(name, args)
        print(out)
        print(err)
        return None 

    t = float(t.group(1))
    nsess = int(nsess.group(1))

    if 'Forged signature is invalid' in out:
        print('INVALID FORGED SIGNATURE')
        print(name, args)
        print(out)
        print(err)
        return None 

    if 'invalid' in out:
        print('SOME SIGNATURE IS INVALID (NOT THE EXTRA ONE)')
        print(name, args)
        print(out)
        print(err)
        return None 

    if 'Decomposition failed' in out:
        return Stats(name, False, t, 0, nsess)

    if 'Attack was successful' not in out:
        print('Did not find "Attack was successful" string in the output, nor the invalid one')
        return None 

    attempt = RE_ATTEMPT.search(out)
    if attempt is None:
        print('Could not match the number of attempts')
        print(name, args)
        print(out)
        print(err)
        return None 

    attempt = int(attempt.group(1))
    return Stats(name, True, t, attempt, nsess)


def to_str(stat: Stats):
    return f'{stat.name},{1 if stat.success else 0},{stat.time:.2f},{stat.attempts},{stat.num_sessions}'


with open('data.csv', 'w') as f:
    f.write('name,success,time,attempts,num_sessions\n')
    with concurrent.futures.ThreadPoolExecutor(max_workers=THREADS) as executor:
        futures = []
        for name, args, nit in PARAMS:
            for _ in range(nit):
                futures.append(executor.submit(run, name, args))

        for future in tqdm(concurrent.futures.as_completed(futures), total=len(futures)):
            data = future.result()
            if data is None:
                break 

            f.write(to_str(data) + '\n')

