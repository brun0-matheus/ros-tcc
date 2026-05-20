import hashlib
import sys 
import time
from sage.modules.free_module_integer import IntegerLattice

load('curves.sage')

if len(sys.argv) != 3:
    print('Usage: sage attack_reduced.sage <GROUP_OPTION> <NUMBER_DIMENSION>')
    print('GROUP_OPTION is an integer from this list:')
    for i in range(len(curves)):
        print(f'  {i}: {curves[i][0]}')
    print('NUMBER_DIMENSION is the maximum dimension for the ros with reduced dimension.')

    exit(1)

opt_curve, num_dim = map(int, sys.argv[1:])
if opt_curve < 0 or opt_curve >= len(curves):
    print('Invalid group option')
    exit(1)

if num_dim <= 1 or num_dim > 10:
    print('Invalid number of dimensions. It has to be an integer in [2, 10]')
    exit(1)

p, E, G = curves[opt_curve][1]()
Zp = GF(p)

def random_oracle(R,m):
    to_hash=str(G.xy()[0])+str(X.xy()[0])+str(R.xy()[0])+m
    hash=hashlib.sha512(to_hash.encode()).digest()
    return Zp(int.from_bytes(hash,"big"))

def verify(message,signature):
    R,s=signature
    c=random_oracle(R,message)
    return G*s==X*c+R

def inner_product(coefficients,values):
    return sum(y*x for x,y in zip(coefficients,values))

def scale_to_Zp(vec):
    assert all([gcd(p,el.denominator())==1 for el in vec])
    return vector(Zp,[Zp(el.numerator())/Zp(el.denominator()) for el in vec])

def pows_gen(n=7,group_bit_len=256,extra_digits=2):
    max_number=2^group_bit_len
    assert n>=2
    pows=[]
    k=n-1
    while k>=1:
        # B=2*k^(2/3)*log(p,k+1)^(1/k) Theorem 2 Bound
        B=1/500

        max_k=ceil(log(max_number,k+1))
        if k==1:
            e_k=0
        else:
            e_k=ceil(log(B*log(p,k+1)*p^((k-1)/k),k+1))+extra_digits

        pows=[(k+1,i) for i in range(e_k,max_k)]+pows
        max_number=(k+1)^e_k
        k-=1

    return pows

def multibase(input_number,pows):
    temp_number=ZZ(input_number)
    digits=[]
    for base in pows[::-1]:
        digits=[temp_number//base]+digits
        temp_number=temp_number%base

    assert inner_product(digits,pows)==input_number
    return digits

# adversary: attack parameters selection
start_time = time.time()

max_basis=num_dim-1
ext_dig=0
factored_pows=pows_gen(
    n=max_basis+1,
    group_bit_len=ceil(log(p,2)), 
    extra_digits=ext_dig
)
pows_bases=[i for i,j in factored_pows]
pows=[i^j for i,j in factored_pows]
ell=len(pows)
print('Number of sessions:', ell)
e_k=[
    min([factored_pows[i][1] if factored_pows[i][0]==k else 1000 for i in range(ell)]) 
    for k in range(2,max_basis+2)
]
I_k=[
    min([i if factored_pows[i][0]==k else 1000 for i in range(ell)])
    for k in range(2,max_basis+2)
]+[ell]

# server: gen pubkey
x=Zp.random_element()
X=G*x

# server: generate commitments
r=[Zp.random_element() for _ in range(ell)]
R=[G*r_i for r_i in r]

# adversary: generate challenges
messages=[f"messages{i}" for i in range(ell)]+["forged_message"]

alpha=[
    [Zp.random_element() for _ in range(pows_bases[i])] 
    for i in range(ell)
]
beta=[Zp.random_element() for i in range(ell)]
blinded_R=[
    [R[i]+G*alpha_i_b+beta[i]*X for alpha_i_b in alpha[i]] 
    for i in range(ell)
]
c=[
    [random_oracle(blinded_R_i_b,messages[i]) for blinded_R_i_b in blinded_R[i]]
    for i in range(ell)
]
qi=[
    [c_i_b-c[i][0] for c_i_b in c[i][1:]]
    for i in range(ell)
]
M=[
    block_matrix([
        [Matrix(ZZ,qi[i])],
        [p*matrix.identity(pows_bases[i]-1)]
    ])
    for i in range(ell)
]

# adversary: estimate quality of lattices
GSO_M=[M[i].gram_schmidt() for i in range(ell)]
quality=[sum([norm(b_star)^2 for b_star in GSO_M[i]]) for i in range(ell)]
rankings=[]
for k in range(max_basis):
    k_rankings=quality[I_k[k]:I_k[k+1]]
    rankings+=[sorted(k_rankings).index(x)+I_k[k] for x in k_rankings]

# adversary: reorder lattices
messages=[messages[i] for i in rankings]+[messages[-1]]
R=[R[i] for i in rankings]
alpha=[alpha[i] for i in rankings]
beta=[beta[i] for i in rankings]
blinded_R=[blinded_R[i] for i in rankings]
c=[c[i] for i in rankings]
qi=[qi[i] for i in rankings]
M=[M[i] for i in rankings]

closest_vectors=[
    IntegerLattice(M[i]).babai([j*pows[i] for j in range(1,pows_bases[i])])
    for i in range(ell)
]
mu=[
    (1/Zp(pows[i]))*scale_to_Zp(M[i].solve_left(closest_vectors[i]))[0] 
    for i in range(ell)
]

# adversary: decomposition of z
attempts=1
while attempts <= 100:
    print(f'Attempt {attempts}...')
    extra_alpha=Zp.random_element()
    R_forge=extra_alpha * inner_product([i*j for i,j in zip(pows,mu)],R)
    c_to_decompose=random_oracle(R_forge,messages[ell])
    NUM_to_decompose= (
        extra_alpha^(-1)*c_to_decompose
        + sum([pows[i]*mu[i]*(-c[i][0]) for i in range(ell)])
        - inner_product(beta,[pows[i]*mu[i] for i in range(ell)])
    )
    
    digits=[0]*ell
    
    for i in range(ell)[::-1]:
        current_digits=multibase(NUM_to_decompose,pows)
        if i!=ell-1 and current_digits[i+1]!=0:
            break
        new_digit=current_digits[i]
        digits[i]=new_digit
        if new_digit>pows_bases[i]:
            break
        if new_digit!=0:
            NUM_to_decompose-=pows[i]*mu[i]*qi[i][new_digit-1]
        if NUM_to_decompose<0:
            break

    if NUM_to_decompose==0:
        break

    attempts+=1

if attempts>100:
    print("Decomposition failed, need to resample the lattices")
else:
    # adversary: sends c to the server
    blinded_c=[c[i][b]+beta[i] for (i,b) in enumerate(digits)]
    blinded_c=[blinded_c[rankings.index(i)] for i in range(ell)]
    
    # server: generate the responses
    s=[blinded_c[i]*x+r[i] for i in range(ell)]
    
    # attacker: generated the forged signatures
    s=[s[i] for i in rankings]
    forged_signatures=[
        (blinded_R[i][digits[i]],s[i]+alpha[i][digits[i]]) 
        for i in range(ell)
    ]
    forged_signatures+=[
        (R_forge,extra_alpha*inner_product([i*j for i,j in zip(pows,mu)],s))
    ]
    
    for i in range(ell):
        if not verify(messages[i], forged_signatures[i]):
            print(f'Signature {i} is invalid.')
            exit(1)

    if not verify(messages[-1], forged_signatures[-1]):
        print('Forged signature is invalid.')
        exit(1)

    print(f'All {ell}+1 signatures are valid. Attack was successful.')
    print('Number of attempts:', attempts)

total_time = time.time() - start_time
print(f'Took {total_time:.2f} seconds.')

