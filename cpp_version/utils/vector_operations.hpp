#include <bits/stdc++.h>

#ifndef VECTOR_OPERATIONS_H
#define VECTOR_OPERATIONS_H

using namespace std;

vector<double> vector_sum(vector<double> vect1, vector<double> vect2) {
    // if vectors have differing dimensionality (should never actually be the case)
    if (vect2.size() > vect1.size()) {
        vect1, vect2 = vect2, vect1;
    }

    for (int i = 0; i < vect2.size(); i++) {
        vect1[i] += vect2[i];
    }

    return vect1;
}

vector<double> vector_sub(vector<double> vect1, vector<double> vect2) {
    // if vectors have differing dimensionality (should never actually be the case)
    if (vect2.size() > vect1.size()) {
        vect1, vect2 = vect2, vect1;
    }

    for (int i = 0; i < vect2.size(); i++) {
        vect1[i] -= vect2[i];
    }

    return vect1;
}

double vector_len(vector<double> vect) {
    double component_squares_sum = 0;
    for (double comp : vect) {
        component_squares_sum += pow(comp, 2);
    }
    return sqrt(abs(component_squares_sum)); // should not need abs but its there just in case
}

vector<double> scale_vector(vector<double> vect, double scalar) {
    for (int i; i < vect.size(); i++ ) {
        vect[i] *= scalar;
    }
    return vect;
}

vector<double> unit_vector(vector<double> vect) {
    double length = vector_len(vect);
    if (length == 0) {
        return vector<double>({0., 0.});
    }
    return scale_vector(vect, 1/length);
}

double dot_product(vector<double> vect1, vector<double> vect2) {
    double dp = 0;
    for (int i; i < vect1.size(); i++) {
        dp += vect1[i] * vect2[i];
    }
    return dp;
}

vector<double> project_vector(vector<double> vect_a, vector<double> vect_b) {
    //Project a onto b and return the resulting scalar projection vector.
    // (unit vector of b) multiplied by (dot product of (unit vector of b) and (vector a))
    vector<double> u_b = unit_vector(vect_b);
    double a1 = dot_product(vect_a, u_b);
    vector<double> proj = scale_vector(u_b, a1);
    return proj;
}

#endif