#include <algorithm>
#include <chrono>
#include <iostream>
#include <queue>
#include <set>
#include <unordered_map>

#define ROUNDS 50000

using namespace std;

typedef priority_queue<pair<double, int>> queue_type;
typedef set<pair<double, int>> set_type;

class timer {
    typedef std::chrono::high_resolution_clock clock_t;
    typedef std::chrono::duration<double, std::ratio<1>> second_t;
    
    std::chrono::time_point<clock_t, second_t> beginning;
    
  public:
    timer(string s) : beginning(clock_t::now()) {
        cout << s << ": ";
    }
    ~timer() {
        cout << elapsed() << " seconds" << endl;
    }
    double elapsed() const {
        return std::chrono::duration_cast<second_t>(clock_t::now() - beginning).count();
    }
};

queue_type q_val;
set_type s_val;
unordered_map<int, double> values;

int N, L;
size_t maxsize;
double factor;
vector<int> order;


template<class T>
void erase(T& sq, int d);

template<>
void erase<queue_type>(queue_type& sq, int d) {
}

template<>
void erase<set_type>(set_type& sq, int d) {
    sq.erase(make_pair(values[d], d));
}

template<class T>
void insert(T& sq, double t, int d) {
    sq.emplace(t, d);
}

template<class T>
void erase_invalid(T& sq);

template<>
void erase_invalid<queue_type>(queue_type& sq) {
    while (values.count(sq.top().second) == 0 or values[sq.top().second] != sq.top().first)
        sq.pop();
}

template<>
void erase_invalid<set_type>(set_type& sq) {
}

template<class T>
int erase_top(T& sq);

template<>
int erase_top<queue_type>(queue_type& sq) {
    int d = sq.top().second;
    sq.pop();
    return d;
}

template<>
int erase_top<set_type>(set_type& sq) {
    int d = sq.rbegin()->second;
    sq.erase(--sq.end());
    return d;
}

template<class T>
void clear(T& sq);

template<>
void clear<queue_type>(queue_type& sq) {
    while (!sq.empty()) {
        sq.pop();
    }
}

template<>
void clear<set_type>(set_type& sq) {
    sq.clear();
}


template<class T>
void round(T& sq, int k) {
    random_shuffle(order.begin(), order.end());
    for (int i=0; i<N; i++) {
        erase(sq, i);
        double t = - factor * i;
        values[order[i]] = t;
        insert(sq, t, order[i]);
        maxsize = max(maxsize, q_val.size());
        erase_invalid(sq);
        if (values.size() > L) {
            values.erase(erase_top(sq));
            erase_invalid(sq);
        }
    }
    clear(sq);
    for (int i=0; i<N; i++) {
        double t = - factor * i - 1;
        values[order[i]] = t;
        insert(sq, t, order[i]);
    }
}

void experiment(int N, int L) {
    ::N = N;
    ::L = L;
    maxsize = 0;
    factor = 1.0 / N;
    for (int i=0; i<N; i++) order.push_back(i);
    cout << "Experiment with N, L = " << N << ", " << L << endl;
    {
        timer _("priority queue");
        for (int i=0; i<ROUNDS; i++)
            round(q_val, i);
    }
    cout << "max container size: " << maxsize << endl;
    values.clear();
    {
        timer _("ordered set");
        for (int i=0; i<ROUNDS; i++)
            round(s_val, i);
    }
    values.clear();
    order.clear();
    clear(s_val);
    clear(q_val);
}

int main() {
    experiment(5,   20); // +35%
    experiment(10,  20); // +49%
    experiment(20,  20); // +45%
    experiment(50,  20); // +34%
    experiment(100, 20); // +26%
    
    experiment(10,  50); // +46%
    experiment(20,  50); // +45%
    experiment(50,  50); // +45%
    experiment(100, 50); // +36%
    experiment(200, 50); // +14%
}

/*
 RESULTS
 
Experiment with N, L = 5, 20
priority queue: 0.189227 seconds
max container size: 10
ordered set: 0.254637 seconds
Experiment with N, L = 10, 20
priority queue: 0.399305 seconds
max container size: 20
ordered set: 0.592998 seconds
Experiment with N, L = 20, 20
priority queue: 0.887089 seconds
max container size: 40
ordered set: 1.28749 seconds
Experiment with N, L = 50, 20
priority queue: 3.3153 seconds
max container size: 70
ordered set: 4.45462 seconds
Experiment with N, L = 100, 20
priority queue: 7.70883 seconds
max container size: 120
ordered set: 9.72131 seconds
 
Experiment with N, L = 10, 50
priority queue: 0.409052 seconds
max container size: 20
ordered set: 0.596094 seconds
Experiment with N, L = 20, 50
priority queue: 0.887655 seconds
max container size: 40
ordered set: 1.28518 seconds
Experiment with N, L = 50, 50
priority queue: 2.4501 seconds
max container size: 100
ordered set: 3.56471 seconds
Experiment with N, L = 100, 50
priority queue: 6.83283 seconds
max container size: 150
ordered set: 9.27302 seconds
Experiment with N, L = 200, 50
priority queue: 16.985 seconds
max container size: 250
ordered set: 19.3366 seconds
 */
