// TODO: all

// keeps a schedule of events, each with three distributions:
//  - distribution in a device, given mean and deviation (shape)
//  - mean distribution among devices (shape and parameters)
//  - deviation distribution among devices (shape and parameters)

// methods:
//  - pop(), which in turn calls
//  - push(event) (private)
