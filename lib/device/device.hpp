// TODO: all

// device implementation, to be subclassed

// data:
// * maximum amount of neighbours allowed
// * maximum export metric value allowed
// * context (messages received from neighbours, possibly including sensor data, and including self value)
// * export (as twin<multitype_map, !SETTING_DUPLICATE_EXPORTS>, true for some real deployments, false for simulation)

// functions:
// * constructor given self ID and export allowances
// * context-dependent operations on fields (self/align/foldhood + share/rep/nbr)
// * body/loop classes proxying to global trace methods (for functions and cycles)

// parameters:
// * M filtering metric export*export -> fully ordered type (based on available sensors)

// direct subclasses should provide:
// * sensors/actuators (interact with simulator or drivers, write on context): position, time, molecules, period...
// * sense() method to fill in current export with sensor data (signature as requested by simulator/handler)

// further subclasses should provide:
// * constructor (signature as requested by simulator/handler)
// * void round() execution function, possibly relying on other functions

// the following system components are outsourced to the simulator/handler:
// * broadcasting of messages
//   > fields received are supposed to be flattened in real deployments (relevant value to default)
// * scheduling of events
