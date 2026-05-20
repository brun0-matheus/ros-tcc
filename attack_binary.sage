import hashlib
import time

# =========================
# === PUBLIC PARAMETERS ===
# =========================

load('curves.sage')

if len(sys.argv) not in [2, 3]:
    print('Usage: sage attack_binary.sage <GROUP_OPTION>')
    print('GROUP_OPTION is an integer from this list:')
    for i in range(len(curves)):
        print(f'  {i}: {curves[i][0]}')

    exit(1)

opt_curve = int(sys.argv[1])
if opt_curve < 0 or opt_curve >= len(curves):
    print('Invalid group option')
    exit(1)

p, E, G = curves[opt_curve][1]()
Zp = GF(p)

# ========================
# === SERVER SIDE CODE ===
# ========================

class Server:
    def __init__(self):
        self._x = Zp.random_element()
        self._X = G * self._x

        self._sess = []

    def get_pubkey(self):
        return self._X

    def commit(self):   # Returns (U, V, sess_id)
        u, v = [Zp.random_element() for _ in range(2)]
        self._sess.append([u, v, True]) 
        return G * u, G * v, len(self._sess) - 1

    def sign(self, sess_id, c, d):   # Returns w
        assert self._sess[sess_id][2], f'Session {i} is not open'
        self._sess[sess_id][2] = False

        z = self._sess[sess_id][0] - c*self._x
        return self._sess[sess_id][1] - d*z

    def total_sessions(self):
        return len(self._sess)


# =================================================
# === COMMON CODE (RANDOM ORACLES AND VERIFIER) ===
# =================================================

def oracle_H(U, V, m):
    to_hash_c = str(G.xy()[0]) + str(U.xy()[0]) + str(V.xy()[0]) + m
    to_hash_d = str(G.xy()[0]) + str(V.xy()[0]) + str(U.xy()[0]) + m
    hash_c = hashlib.sha512(to_hash_c.encode()).digest()
    hash_d = hashlib.sha512(to_hash_d.encode()).digest()
    
    c = Zp(int.from_bytes(hash_c, 'big'))
    d = Zp(int.from_bytes(hash_d, 'big'))
    assert c != d 
    return c, d
    
def verify(pubkey, message, signature):
    U, V, w = signature
    c, d = oracle_H(U, V, message)
    H = U - c * pubkey
    V2 = G * w + H * d
    
    return V2 == V


# ==========================
# === ATTACKER SIDE CODE ===
# ==========================

def inner_product(coefficients, values):
    return sum(a*b for a, b in zip(coefficients, values))

start_time = time.time()

server = Server()
X = server.get_pubkey()

# open 'ell' sessions
ell = p.bit_length()
print('Number of sessions:', ell)
messages = [f"message{i}" for i in range(ell)] + ["forged message"]

# generate commitments
commits = [server.commit() for _ in range(ell)]
U = [comm[0] for comm in commits]
V = [comm[1] for comm in commits]

# generate random blind d's
blind_d = [Zp.random_element() for i in range(ell)]

# calculate V's of normal signatures
blind_V = [V[i] - U[i]*blind_d[i] for i in range(ell)]

# calculate U's of normal signatures
blind_ulog = [[Zp.random_element() for b in [0,1]] for i in range(ell)]
blind_U = [[u * G for u in par] for par in blind_ulog]

# generate challenges c's and d's
par_cd = [
    [oracle_H(blind_U[i][b], blind_V[i], messages[i]) for b in [0,1]]
    for i in range(ell)
]

# generate blind c's
blind_c = [
    [par_cd[i][b][0] * par_cd[i][b][1] / blind_d[i] for b in [0,1]]
    for i in range(ell)
]

# generate linear combination and parameters of forged signature

# P(ep[i][b]) = 2^i * b
ep = [[blind_c[i][b] * blind_d[i] for b in [0, 1]] for i in range(ell)]
p0 = -sum([Zp(2)^i * ep[i][0]/(ep[i][1] - ep[i][0]) for i in range(ell)])
P = ([p0] + [Zp(2)^i / (ep[i][1] - ep[i][0]) for i in range(ell)])

prodV = inner_product(P[1:], V)
prodU = inner_product(P[1:], [blind_d[i] * U[i] for i in range(ell)])

targV = prodV - prodU
targulog = Zp.random_element()
targU = targulog * G

targc, targd = oracle_H(targU, targV, messages[-1])

# calculate the binary decomposition and send blind cs accordingly

y_to_dec = targc*targd + P[0]
bits = [int(b) for b in bin(y_to_dec)[2:].rjust(ell, '0')][::-1]
send_blind_c = [blind_c[i][b] for (i, b) in enumerate(bits)]

# receive the responses
w = [server.sign(i, send_blind_c[i], blind_d[i]) for i in range(ell)]

# generate the signatures

# normal signatures first
signatures = [
    (blind_U[i][bits[i]], blind_V[i],
     w[i] - par_cd[i][bits[i]][1] * blind_ulog[i][bits[i]])
    for i in range(ell)
]

# forged signature
signatures += [(targU, targV, inner_product(P[1:], w) - targd * targulog)]

for i in range(ell):
    if not verify(X, messages[i], signatures[i]):
        print(f'Signature {i} is invalid.')
        exit(1)

if not verify(X, messages[-1], signatures[-1]):
    print('Forged signature is invalid.')
    exit(1)

print(f'All {ell}+1 signatures are valid. Attack was successful.')
print('Number of attempts: 1')

assert len(messages) == ell + 1
assert len(messages) == len(signatures)

total_time = time.time() - start_time
print(f'Took {total_time:.2f} seconds.')
