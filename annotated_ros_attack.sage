# Reconstructed and commented SageMath implementation from Appendix A of the paper
# "Dimensional eROSion: Improving the ROS Attack with Decomposition in Higher Bases"
#
# Purpose:
#   This script demonstrates the concrete attack against the one-more unforgeability
#   of Schnorr blind signatures using the higher-base ROS decomposition described in
#   the paper.
#
# Notes:
#   - The PDF appendix text is OCR-garbled in a few places; this is a cleaned
#     reconstruction of the code structure and intent.
#   - The script is written for SageMath, not plain Python.
#   - Several parameters are tuned for the secp256k1 curve and the concrete
#     experimental setup from the paper.

import hashlib
from sage.modules.free_module_integer import IntegerLattice

# ---------------------------------------------------------------------------
# Public curve parameters: secp256k1
# ---------------------------------------------------------------------------
# q = field prime for the base field F_q
q = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F
Zq = GF(q)

# Elliptic curve: y^2 = x^3 + 7 over F_q
E = EllipticCurve(Zq, [0, 7])

# Standard generator of secp256k1
G = E(
    0x79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798,
    0x483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8,
)

# The group order p is the modulus used by the ROS attack
E.set_order(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141)
p = G.order()
Zp = GF(p)

# ---------------------------------------------------------------------------
# Random oracle and verification
# ---------------------------------------------------------------------------
def random_oracle(R, m):
    """
    Hash the x-coordinates of the public generator, the public key X,
    the commitment R, and the message m into Z_p.

    This models the Fiat-Shamir-style random oracle used in the paper.
    """
    to_hash = str(G.xy()[0]) + str(X.xy()[0]) + str(R.xy()[0]) + m
    digest = hashlib.sha512(to_hash.encode()).digest()
    return Zp(int.from_bytes(digest, "big"))


def verify(message, signature):
    """
    Verify a Schnorr-style signature (R, s) on 'message' against public key X.
    The verification equation is:
        s*G = R + c*X,
    where c is derived from the random oracle.
    """
    R, s = signature
    c = random_oracle(R, message)
    assert G * s == X * c + R, "verification equation fails"
    return True


# ---------------------------------------------------------------------------
# Helper routines
# ---------------------------------------------------------------------------
def inner_product(coefficients, values):
    """
    Standard dot product:
        sum(coefficients[i] * values[i])
    """
    return sum(y * x for x, y in zip(coefficients, values))


def scale_to_Zp(vec):
    """
    Convert a rational vector to a vector over Z_p by multiplying numerator and
    denominator modulo p.

    The code asserts that all denominators are invertible mod p.
    """
    assert all(gcd(p, el.denominator()) == 1 for el in vec)
    return vector(Zp, [Zp(el.numerator()) / Zp(el.denominator()) for el in vec])


# ---------------------------------------------------------------------------
# Mixed-radix power construction
# ---------------------------------------------------------------------------
def pows_gen(n=7, group_bit_len=256, extra_digits=2):
    """
    Build the list of mixed-radix place values used by the attack.

    The paper decomposes a target integer using bases 2, 3, ..., n.
    For each base (k+1), this function chooses how many digit positions are used
    before moving to the next smaller base.

    Returns a list of pairs (base, exponent).
    """
    max_number = 2 ** group_bit_len
    assert n >= 2

    pows = []
    k = n - 1

    while k >= 1:
        # Concrete tuning parameter used in the implementation.
        # In the paper this is a practical simplification of the asymptotic formula.
        B = 1 / 500

        max_k = ceil(log(max_number, k + 1))

        if k == 1:
            e_k = 0
        else:
            e_k = ceil(log(B * log(p, k + 1) * p ** ((k - 1) / k), k + 1)) + extra_digits

        # Add all powers for this base, from exponent e_k up to max_k - 1.
        pows = [(k + 1, i) for i in range(e_k, max_k)] + pows

        # Update the remaining range for the next smaller base.
        max_number = (k + 1) ** e_k
        k -= 1

    return pows


def multibase(input_number, pows):
    """
    Convert 'input_number' into digits for the mixed-radix system described by
    'pows'.

    The function walks through the place values from largest to smallest and
    recovers each digit by repeated division/remainder.
    """
    temp_number = ZZ(input_number)
    digits = []

    for base in pows[::-1]:
        digits = [temp_number // base] + digits
        temp_number = temp_number % base

    assert inner_product(digits, pows) == input_number
    return digits


# ---------------------------------------------------------------------------
# Attack parameters and session setup
# ---------------------------------------------------------------------------
max_basis = 7
ext_dig = 0

# Generate mixed-radix place values for a 256-bit target.
factored_pows = pows_gen(
    n=max_basis + 1,
    group_bit_len=ceil(log(p, 2)),
    extra_digits=ext_dig,
)

# Separate base labels from actual numeric place values.
pows_bases = [i for i, j in factored_pows]
pows = [i ** j for i, j in factored_pows]
ell = len(pows)
print(f'Number of sessions: {ell}')

# For each base k, remember where that base starts in the global list.
e_k = [
    min([factored_pows[i][1] if factored_pows[i][0] == k else 1000 for i in range(ell)])
    for k in range(2, max_basis + 2)
]
I_k = [
    min([i if factored_pows[i][0] == k else 1000 for i in range(ell)])
    for k in range(2, max_basis + 2)
] + [ell]

# ---------------------------------------------------------------------------
# Server side: key generation and commitments
# ---------------------------------------------------------------------------
x = Zp.random_element()   # secret signing key
X = G * x                 # public key

r = [Zp.random_element() for _ in range(ell)]  # per-session nonces
R = [G * r_i for r_i in r]                     # public commitments

# ---------------------------------------------------------------------------
# Adversary side: choose messages and blind them in many ways
# ---------------------------------------------------------------------------
messages = [f"messages {i}" for i in range(ell)] + ["forged_message"]

# For each session, the adversary prepares one blinding factor per allowed digit
# in that position, plus an extra beta term used to align responses later.
alpha = [[Zp.random_element() for _ in range(pows_bases[i])] for i in range(ell)]
beta = [Zp.random_element() for i in range(ell)]

# For each session and each possible digit, create a blinded commitment.
blinded_R = [
    [R[i] + G * alpha_i_b + beta[i] * X for alpha_i_b in alpha[i]]
    for i in range(ell)
]

# Query the oracle on every blinded commitment.
c = [
    [random_oracle(blinded_R_i_b, messages[i]) for blinded_R_i_b in blinded_R[i]]
    for i in range(ell)
]

# For each session, form the differences c_b - c_0.
# These differences define a small lattice whose CVP solution gives the scaling mu.
qi = [[c_i_b - c[i][0] for c_i_b in c[i][1:]] for i in range(ell)]

# Build the lattice basis used in the CVP step.
# The top row is the vector of differences q_i, and the lower rows are p times
# the identity. This forces the lattice to encode modular relations.
M = [
    block_matrix([
        [Matrix(ZZ, qi[i])],
        [p * matrix.identity(pows_bases[i] - 1)],
    ])
    for i in range(ell)
]

# ---------------------------------------------------------------------------
# Optional quality estimation and lattice reordering
# ---------------------------------------------------------------------------
# The paper suggests estimating lattice "quality" from the Gram-Schmidt basis
# length and placing the worst lattices at the highest digit positions.
GSO_M = [M[i].gram_schmidt() for i in range(ell)]
quality = [sum([norm(b_star) ** 2 for b_star in GSO_M[i]]) for i in range(ell)]

rankings = []
for k in range(max_basis):
    k_rankings = quality[I_k[k]:I_k[k + 1]]
    rankings += [sorted(k_rankings).index(x) + I_k[k] for x in k_rankings]

# Reorder all session data consistently.
messages = [messages[i] for i in rankings] + [messages[-1]]
R = [R[i] for i in rankings]
alpha = [alpha[i] for i in rankings]
beta = [beta[i] for i in rankings]
blinded_R = [blinded_R[i] for i in rankings]
c = [c[i] for i in rankings]
qi = [qi[i] for i in rankings]
M = [M[i] for i in rankings]

# ---------------------------------------------------------------------------
# Solve the CVP instances and recover the scaling factors mu_i
# ---------------------------------------------------------------------------
closest_vectors = [
    IntegerLattice(M[i]).babai([j * pows[i] for j in range(1, pows_bases[i])])
    for i in range(ell)
]

# mu_i is the scalar that makes the approximate lattice relation line up with the
# target powers. The first coordinate after solving the lattice relation gives it.
mu = [
    (1 / Zp(pows[i])) * scale_to_Zp(M[i].solve_left(closest_vectors[i]))[0]
    for i in range(ell)
]

# ---------------------------------------------------------------------------
# Main decomposition loop
# ---------------------------------------------------------------------------
# The algorithm repeats until it finds a decomposition of the derived target
# c_to_decompose into the mixed-radix digit pattern supported by the chosen mu_i.
attempts = 0

while True:
    attempts += 1

    # Randomize the final linear combination by choosing a fresh alpha.
    extra_alpha = Zp.random_element()

    # Build the forged commitment for the final session.
    # The weighted sum uses the mu_i coefficients to combine the earlier R_i.
    R_forge = extra_alpha * inner_product([i * j for i, j in zip(pows, mu)], R)

    # Oracle value for the forged session.
    c_to_decompose = random_oracle(R_forge, messages[ell])

    # This is the integer whose mixed-radix decomposition we want to recover.
    # The formula matches the paper: remove the known offsets contributed by the
    # session-wise blindings and the q_i differences.
    NUM_to_decompose = (
        extra_alpha ** (-1) * c_to_decompose
        + sum([pows[i] * mu[i] * (-c[i][0]) for i in range(ell)])
        - inner_product(beta, [pows[i] * mu[i] for i in range(ell)])
    )

    # digits[i] will eventually hold the selected digit for session i.
    digits = [0] * ell

    # Recover digits from high to low.
    for i in range(ell)[::-1]:
        current_digits = multibase(NUM_to_decompose, pows)

        # If we are not at the last digit, the next higher digit must already be 0
        # for the decomposition to remain valid.
        if i != ell - 1 and current_digits[i + 1] != 0:
            break

        new_digit = current_digits[i]
        digits[i] = new_digit

        # Digits cannot exceed the number of available blinding choices.
        if new_digit > pows_bases[i]:
            break

        # Subtract the contribution of the chosen digit.
        if new_digit != 0:
            NUM_to_decompose -= pows[i] * mu[i] * qi[i][new_digit - 1]

        if NUM_to_decompose < 0:
            break

    if NUM_to_decompose == 0:
        break

    if attempts > 98:
        break

if attempts > 98:
    print("Decomposition failed, need to resample the lattices")
else:
    # -------------------------------------------------------------------
    # Final forgery phase
    # -------------------------------------------------------------------
    # Convert the chosen digits back into the proper blinded challenges.
    blinded_c = [c[i][b] + beta[i] for (i, b) in enumerate(digits)]
    blinded_c = [blinded_c[rankings.index(i)] for i in range(ell)]

    # Server responses for each session: s_i = c_i * x + r_i
    s = [blinded_c[i] * x + r[i] for i in range(ell)]
    s = [s[i] for i in rankings]

    # Reconstruct signatures for all opened sessions.
    forged_signatures = [
        (blinded_R[i][digits[i]], s[i] + alpha[i][digits[i]])
        for i in range(ell)
    ]

    # Forge the extra final signature using the weighted sum.
    forged_signatures += [
        (R_forge, extra_alpha * inner_product([i * j for i, j in zip(pows,mu)], s))
    ]

    # Check that every forged signature verifies.
    print(all([verify(messages[i], forged_signatures[i]) for i in range(ell + 1)]))
