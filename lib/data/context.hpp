// TODO: all

// class context:
//  - keeps associations device -> export
//  - export are added to contexts at end of rounds
//  - old/irrelevant exports are filtered at start of rounds

// based on:
//  - exports
//  - field

// parameters:
//  - Ts... types supported by the exports
//  - T executable function for filtering exports

// members:
//  - map<size_t, shared_ptr<exports<Ts...>>>
//  - size_t self

// methods:
//  - insert(size_t, shared_ptr<...>): inserts exports for a device
//  - filter(): filters unwanted exports
//  - nbr<A>(trace_t, def): returns field<A> of data from aligned neighbours
//  - old<A>(trace_t, def): returns old data (aligned)
//  - aligned(trace_t): returns set of devices with specified code point
