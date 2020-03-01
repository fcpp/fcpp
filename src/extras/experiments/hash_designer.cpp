#include <cmath>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <vector>

#define LENGTH 50
#define POINTS 200
#define TRIES  10000

using namespace std;

mt19937_64 rng(42);
uniform_real_distribution<double> unif(0, 1.0/POINTS/TRIES);

vector<double> goodness(double f, int len) {
    vector<double> g;
    set<double> mul;
    mul.insert(0);
    mul.insert(1);
    g.push_back(1);
    for (int i=1; i<len; i++) {
        double m = i*f - (int)(i*f);
        g.push_back(min({g.back(), m - *(--mul.upper_bound(m)), *mul.upper_bound(m) - m}));
        mul.insert(m);
    }
    return g;
}

double measure(const vector<double>& g) {
    double m = 1;
    for (int i=1; i<g.size(); i++) m = min(m, (i+1)*g[i]);
    return m;
}

ostream& operator<<(ostream& o, const vector<double>& g) {
    for (int i=0; i<g.size(); i++) {
        o << " " << g[i]; //*(i+1);
    }
    return o;
}

void plot(int length) {
    vector<double> mn, mx;
    for (int i=0; i<POINTS; i++) {
        mn.push_back(1);
        mx.push_back(0);
        for (int j=0; j<TRIES; j++) {
            double r = i*1.0/POINTS + j*1.0/POINTS/TRIES + unif(rng);
            r = measure(goodness(r, length));
            mn.back() = min(mn.back(), r);
            mx.back() = max(mx.back(), r);
        }
    }
    cout << "p = (0,0)";
    for (int i=0; i<POINTS; i++)
        cout << " -- (" << (i+0.5)/POINTS << "," << mn[i] << ")";
    cout << " -- (1,0) -- cycle;\n";
    cout << "q = (0,0)";
    for (int i=0; i<POINTS; i++)
        cout << " -- (" << (i+0.5)/POINTS << "," << mx[i] << ")";
    cout << " -- (1,0) -- cycle;\n";
    cout << "fill(q, black);\n";
    cout << "fill(p, mediumgray);\n";
}

void plot() {
    cout << setprecision(3) << fixed;
    cout << "unitsize(10cm);\n";
    cout << "path p,q;\n";
    for (int i=10; i<=LENGTH; i+=10) {
        plot(i);
        cout << "newpage();\n";
    }
}

double best(int length) {
    double mx = 0, val = 0;
    for (int i=0; i<POINTS/2; i++) {
        for (int j=0; j<TRIES; j++) {
            double r = i*1.0/POINTS + j*1.0/POINTS/TRIES + unif(rng);
            double m = measure(goodness(r, length));
            if (m > mx) {
                mx = m;
                val = r;
            }
        }
    }
    return val;
}

void best() {
    for (int i=5; i<=LENGTH; i+=5) {
        double r = best(i);
        cout << i << ": " << r << endl; // << goodness(r, i)
    }
}

double bestsearch(double min, double max, int length, int tries) {
    double mx = 0, val = 0;
    for (int i=0; i<tries; i++) {
        double r = min + (i*1.0/tries + unif(rng)*POINTS*TRIES/tries)*(max-min);
        double m = measure(goodness(r, length));
        if (m > mx) {
            mx = m;
            val = r;
        }
    }
    return val;
}

void bestsearch(int length) {
    cout << "LENGTH " << length << endl;
    double r;
    r = bestsearch(0.2763, 0.2766, length, POINTS*TRIES/length);
    cout << r << ":\t" << measure(goodness(r, length)) << endl;
    r = bestsearch(0.3818, 0.3824, length, POINTS*TRIES/length);
    cout << r << ":\t" << measure(goodness(r, length)) << endl;
    r = bestsearch(0.4197, 0.4199, length, POINTS*TRIES/length);
    cout << r << ":\t" << measure(goodness(r, length)) << endl;
}

void bestsearch() {
    for (int l=10; l<=1000; l*=10) {
        bestsearch(1*l);
        bestsearch(2*l);
        bestsearch(5*l);
    }
}

double refine(double min, double max, int length) {
    int T = 10000;
    for (int i=0; i<15; i++) {
        double r = bestsearch(min, max, length, T);
        min = r - (r-min)/10;
        max = r + (max-r)/10;
    }
    return (min+max)/2;
}

void refine(int length) {
    cout << "LENGTH " << length << endl;
    double r;
    r = refine(0.2763, 0.2766, length);
    cout << r << ":\t" << measure(goodness(r, length)) << endl;
    r = refine(0.3818, 0.3824, length);
    cout << r << ":\t" << measure(goodness(r, length)) << endl;
    r = refine(0.4197, 0.4199, length);
    cout << r << ":\t" << measure(goodness(r, length)) << endl;
}

void refine() {
    cout << setprecision(17) << fixed;
    for (int l=10; l<=1000; l*=10) {
        refine(1*l);
        refine(2*l);
        refine(5*l);
    }
    refine(10000);
}

void display(double f) {
    cout << f << ": " << measure(goodness(f, 10000)) << "|" << goodness(f, 20) << endl;
}

vector<double> goodness(uint64_t f, uint64_t m, int len) {
    vector<double> g;
    set<uint64_t> mul;
    uint64_t pow = 1;
    mul.insert(1);
    mul.insert(m);
    g.push_back(m);
    for (int i=1; i<len; i++) {
        pow = (pow * f) % m;
        g.push_back(min({g.back(), pow * 1.0 / *(--mul.upper_bound(pow)), *mul.upper_bound(pow) * 1.0 / pow}));
        mul.insert(pow);
    }
    return g;
}

void discrete(uint64_t m, int len) {
    uniform_int_distribution<uint64_t> unif(1, m/2);
    vector<vector<double>> gbest(len);
    vector<double> vbest(len, 0.0);
    vector<uint64_t> fbest(len, 0);
    
    for (int i=0; i<POINTS*TRIES; ++i) {
        uint64_t r = 2*i+1; //unif(rng)*2 - 1;
        vector<double> g = goodness(r, m, len);
        double v = g[0];
        for (int i=1; i<len; i++) {
            v = min(v, pow(g[i], i+1));
            if (v > vbest[i]) {
                gbest[i] = g;
                vbest[i] = v;
                fbest[i] = r;
            }
        }
    }
    for (int i=1; i<len; i++)
        cout << fbest[i] << ": " << vbest[i] << "|" << gbest[i] << endl;
}

int main() {
    discrete(1L<<34, 6);
}
