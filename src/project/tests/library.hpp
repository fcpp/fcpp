#ifndef FCPP_COMMON_TAGGED_TUPLE_H_
#define FCPP_COMMON_TAGGED_TUPLE_H_

void push_back(int n);

template <typename... Ts>
void push_back() {
    push_back(sizeof...(Ts));
}

int pop_back();

#endif
