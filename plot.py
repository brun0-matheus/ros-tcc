import matplotlib.pyplot as plt
from collections import namedtuple
from math import sqrt

Algo = namedtuple('Algo', ['type', 'curve'])
MAX_DIM = 0
VERY_BIG_DIM_LIMIT = 100
COLORS = 'rgbkcmy'
all_curves = set()

def algo_from_name(name: str) -> Algo:
    t,c,d = name.strip().split('_')
    return Algo(t,c), int(d)

times, success, attempts, sessions, cnt = [dict() for _ in range(5)]
times_list = dict()

with open('data.csv', 'r') as f:
    assert f.readline() == 'name,success,time,attempts,num_sessions\n'
    for li in f:
        name, suc, t, attempt, sess = li.strip().split(',')
        algo, dim = algo_from_name(name)
        suc, attempt, sess = map(int, [suc, attempt, sess])
        t = float(t)

        if algo not in times:
            times[algo] = [0] * VERY_BIG_DIM_LIMIT  
            success[algo] = [0] * VERY_BIG_DIM_LIMIT
            attempts[algo] = [0] * VERY_BIG_DIM_LIMIT
            sessions[algo] = [0] * VERY_BIG_DIM_LIMIT
            cnt[algo] = [0] * VERY_BIG_DIM_LIMIT
            times_list[algo] = [list() for _ in range(VERY_BIG_DIM_LIMIT)]

        times[algo][dim] += t 
        success[algo][dim] += suc 
        attempts[algo][dim] += attempt
        sessions[algo][dim] += sess 
        cnt[algo][dim] += 1
        times_list[algo][dim].append(t)

        MAX_DIM = max(MAX_DIM, dim)

        all_curves.add(algo.curve)


curve_colors = {c: COLORS[i] for i, c in enumerate(all_curves)}

CNT = None 
for _, c in cnt.items():
    if CNT is None:
        CNT = c[0]

    assert all(x == CNT or x == 0 for x in c)

# Time 
x = [0] + list(range(2, MAX_DIM+1))
for algo, t in times.items():
    type, curve = algo
    y = t[:1] + t[2:MAX_DIM+1]
    y = [v/CNT for v in y]

    line = '-' if type == 'cpp' else '--'
    plt.plot(x, y, line + curve_colors[curve], label=f'{curve} ({type})')

plt.ylabel('Time (s)')
plt.xlabel('Number of dimensions')
plt.title('Average running time over multiple curves')
plt.legend()
plt.show()

# Success rate
x = list(range(3, MAX_DIM + 1))
for algo, suc in success.items():
    type, curve = algo
    assert suc[0] == CNT 
    assert suc[2] == CNT 

    y = suc[3:MAX_DIM+1]
    y = [v/CNT for v in y]

    line = '-' if type == 'cpp' else '--'
    plt.plot(x, y, line + curve_colors[curve], label=f'{curve} ({type})')

plt.ylabel('Success rate')
plt.xlabel('Number of dimensions')
plt.title('Success rate over multiple curves')
plt.legend()
plt.show()

# Big table 

def print_table(type):
    print(f'Table for {type}:')
    curves = list(all_curves)
    curves.sort()

    print(r'''\begin{table}[]
\begin{tabular}{lllll}
\textbf{Curve} & \textbf{Maximum dimension} & \textbf{Time (s)} & \textbf{Success Rate} & \textbf{Number of Sessions} \\
''')
    for curve in curves:
        algo = Algo(type, curve)
        for ndim in [0] + list(range(2, MAX_DIM+1)):
            t = times[algo][ndim] / CNT 
            s = success[algo][ndim] / CNT * 100
            nsess = sessions[algo][ndim]
            assert nsess % CNT == 0
            nsess //= CNT 
            dev = sqrt(sum((ti - t)**2 for ti in times_list[algo][ndim]) / CNT)

            print(f'{curve} & {ndim} & {t:.2f} \\pm {dev:.2f} & {s:.1f} \\% & {nsess} \\\\')
    print('\\end{tabular}\n\\end{table}')


print_table('cpp')
print_table('sage')
