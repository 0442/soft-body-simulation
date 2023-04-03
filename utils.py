from math import sqrt
from functools import reduce

def num_sign(num:float|int) -> int:
        if num == 0:
                return 0
        sign = int(num/abs(num))
        return sign

def vector_sum(vect1:tuple[int|float], vect2:tuple[int|float]):
        v1, v2 = list(vect1), list(vect2)
        if len(v2) > len(v1):
                v1, v2 = v2, v1

        for i in range(len(v2)):
                v1[i] += v2[i]

        return tuple(v1)

def vector_sub(vect1:tuple[int|float], vect2:tuple[int|float]):
        v1, v2 = list(vect1), list(vect2)
        if len(v2) > len(v1):
                v1, v2 = v2, v1

        for i in range(len(v2)):
                v1[i] -= v2[i]

        return tuple(v1)

def vector_len(vector:tuple[float]):
        v = list(vector)
        s = reduce(lambda s,i: s+i**2, v, 0) ** (1/2)
        return s

def unit_vec(vector:tuple[float]):
        m = 1 / vector_len(vector)
        n = []
        for i in vector:
                n.append(i * m)
        return n

def scale_vector(vector:tuple[float], scalar:float):
        v = list(vector)
        n = []
        for i in v:
                n.append(i * scalar)
        return n