#include <bits/stdc++.h>

#ifndef VECTOR_OPERATIONS_H
#define VECTOR_OPERATIONS_H

using namespace std;

template <typename _T = double>
vector<_T> vector_sum(vector<_T> vect1, vector<_T> vect2)
{
    // if vectors have differing dimensionality (should never actually be the case)
    if (vect2.size() > vect1.size())
        vect1, vect2 = vect2, vect1;

    for (int i = 0; i < vect2.size(); i++)
        vect1[i] += vect2[i];

    return vect1;
}

template <typename _T = double>
vector<_T> vector_sub(vector<_T> vect1, vector<_T> vect2)
{
    // if vectors have differing dimensionality (should never actually be the case)
    if (vect2.size() > vect1.size())
        vect1, vect2 = vect2, vect1;

    for (int i = 0; i < vect2.size(); i++)
        vect1[i] -= vect2[i];

    return vect1;
}

template <typename _T = double>
double vector_len(vector<_T> vect)
{
    _T component_squares_sum = 0;
    for (_T comp : vect)
        component_squares_sum += pow(comp, 2);

    return (double)sqrt(component_squares_sum);
}

template <typename _T = double>
vector<_T> scale_vector(vector<_T> vect, _T scalar)
{
    for (int i; i < vect.size(); i++)
        vect[i] *= scalar;

    return vect;
}

template <typename _T = double>
vector<_T> unit_vector(vector<_T> vect)
{
    double length = vector_len(vect);
    if (length == 0)
        return vector<double>({0., 0.});

    return scale_vector(vect, 1 / length);
}

template <typename _T = double>
double dot_product(vector<_T> vect1, vector<_T> vect2)
{
    double dp = 0;
    for (int i; i < vect1.size(); i++)
        dp += vect1[i] * vect2[i];

    return dp;
}

template <typename _T = double>
vector<_T> project_vector(vector<_T> vect_a, vector<_T> vect_b)
{
    // Project a onto b and return the result.
    //  (unit vector of b) multiplied by (dot product of (unit vector of b) and (vector a))
    vector<_T> u_b = unit_vector<_T>(vect_b);
    double a1 = dot_product<_T>(vect_a, u_b);
    vector<_T> proj = scale_vector<_T>(u_b, a1);
    return proj;
}

#endif