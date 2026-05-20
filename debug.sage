from collections import namedtuple
from hashlib import sha256

prime_curve = 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f
E = EllipticCurve(GF(prime_curve), (0, 7))
G = E(0x79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798, 0x483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8)
p = 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141 * 0x1
E.set_order(p)
K = GF(p)

def str_gp(X):
    return f'({X.x()}, {X.y()})'

def calc_hash_el(name, X, U, V, msg):
    s = str_gp(X) + str_gp(U) + str_gp(V) + msg + name 
    #print(s)
    digest = sha256(s.encode()).digest()
    #print(digest.hex())
    return K(int.from_bytes(digest, 'little'))


def calc_hash(X, U, V, msg):
    return calc_hash_el('c', X, U, V, msg), calc_hash_el('d', X, U, V, msg)


'''
X = E(55768883347174700701128593822531923655269564037185407950623971712327066762764, 5179869807602915274802836544614511524675144207370922289314588580177536126057)
U = E(51092224309035482010739810879876339658436723075701848183164328212409105550104, 38409885166170612582546171944329765702776949288668025194542724060626587842684)
V = E(29839810571289043338746837365348492786484131224129349499153261869958908809211, 26119392053788762043927482929542635056461929080224851612893493866933675449147)
c = K(54422816019857957928773062542032512991704853556112996110346848570481687966029)
d = K(28084016384220456773367282865616399171943904736937289975077363745090572046617)

slac, slad = calc_hash(X, U, V, 'Msg 0')
assert slac == c 
assert slad == d 

print('Ok')
'''

SessCommit = namedtuple('SessCommit', ['u', 'v'])
SessFim = namedtuple('SessFim', ['c', 'd', 'w'])

ell = 209 
sess_commit = [0] * ell 
sess_fim = [0] * ell 


load('debug_deu_errado.sage')

privkey = K(privkey)
sess_commit = [SessCommit(*map(K, x)) for x in sess_commit]
sess_fim = [SessFim(*map(K, x)) for x in sess_fim]
coefs = [K(x) for x in coefs]
forgw, forgu, forgc, forgd = map(K, [forgw, forgu, forgc, forgd])
X = E(X)
forgU = E(forgU)
forgV = E(forgV)

for i in range(ell):
    u, v = sess_commit[i]
    c, d, w = sess_fim[i]
    assert w == v - d*u + c*d*privkey


forgv = 0
for i in range(ell):
    forgv += coefs[i] * (sess_commit[i].v - sess_fim[i].d * sess_commit[i].u)

assert X == privkey * G 
assert forgU == forgu * G 
assert forgV == forgv * G

slac, slad = calc_hash(X, U, V, 'Msg -1337')
assert forgc == slac 
assert forgd == slad

