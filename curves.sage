# Each functions returns: (curve_order, curve, generator)

def p224():
    p = 0xffffffffffffffffffffffffffffffff000000000000000000000001
    K = GF(p)
    a = K(0xfffffffffffffffffffffffffffffffefffffffffffffffffffffffe)
    b = K(0xb4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4)
    E = EllipticCurve(K, (a, b))
    G = E(0xb70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21, 0xbd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34)
    q = 0xffffffffffffffffffffffffffff16a2e0b8f03e13dd29455c5c2a3d * 0x1
    E.set_order(q)
    return q, E, G 

def p256():
    p = 0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff
    K = GF(p)
    a = K(0xffffffff00000001000000000000000000000000fffffffffffffffffffffffc)
    b = K(0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b)
    E = EllipticCurve(K, (a, b))
    G = E(0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296, 0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5)
    q = (0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551 * 0x1)
    E.set_order(q)
    return q, E, G

def p384():
    p = 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffeffffffff0000000000000000ffffffff
    K = GF(p)
    a = K(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffeffffffff0000000000000000fffffffc)
    b = K(0xb3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875ac656398d8a2ed19d2a85c8edd3ec2aef)
    E = EllipticCurve(K, (a, b))
    G = E(0xaa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a385502f25dbf55296c3a545e3872760ab7, 0x3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c00a60b1ce1d7e819d7a431d7c90ea0e5f)
    q = (0xffffffffffffffffffffffffffffffffffffffffffffffffc7634d81f4372ddf581a0db248b0a77aecec196accc52973 * 0x1)
    E.set_order(q)
    return q, E, G

def p521():
    p = 0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
    K = GF(p)
    a = K(0x01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    b = K(0x0051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef109e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00)
    E = EllipticCurve(K, (a, b))
    G = E(0x00c6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3dbaa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66, 0x011839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e662c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650)
    q = (0x01fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409 * 0x1)
    E.set_order(q)
    return q, E, G

def secp256k1():
    q=0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f
    Zq=GF(q)
    E=EllipticCurve(Zq,[0,7])
    G=E(0x79be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798,0x483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8)
    E.set_order(0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141)
    p=G.order()
    Zp=GF(p)

    return p, E, G 

curves = [
    ('P224', p224),
    ('P256', p256),
    ('P384', p384),
    ('P521', p521),
    ('secp256k1', secp256k1)
]

