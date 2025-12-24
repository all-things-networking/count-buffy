## Table of Contents
- [Background and Motiviation 🧛‍♀️](#count-buffy-️)
- [Description of Experiments](#experiments)
- [Hello World Example](#getting-started)
- [Running the Experiments to Reproduce the Results](#experiments-1)
- [How 🧛‍♀️is Implemented](#walkthrough-of-the-prio-case-study)

# Count Buffy 🧛‍♀️

Count Buffy (🧛‍♀️) is a verification tool for performance verification of packet
schedulers in the network.

Terminology:

- Buffer: A fifo queue of packets with finite capacity
- Packet Stream: A sequence of packets
- Packet Scheduler: A processing unit that reads packet from a set of input Buffers and
  produces a set of output packet streams
- Query: A predicate over output packet stream
- Workload: A predicate over input packet stream

On a high level, input and output of a packet scheduler is a set of packet streams.
Input packet streams are fed into Buffers and scheduler reads the buffered packets, and don't
directly have access to the input stream.
Scheduler the processes the packets and produces output stream accordingly.
Workload/Query is an abstract representation of a set of inputs/outputs to/from the scheduler.

Queries are performance properties, i.e., they specify performance related properties of
the output like throughput.

The goal of verification task is to verify whenever we fed traffic consistent with the workload
into the scheduler, the output of the scheduler satisfies the Query.

For instance, assume that we have a rate limiter that limits the output traffic rate to
1 packet per two timesteps.
Our goal is to verify that even if we feed consistent traffic of 1 packet every timestep into
the network, we see at most one packet every two timesteps in the output.

Without verification, we need to manually construct some input traffic with these specification,
feed them into the network, and then check if the output is rate-limited.

With verification, we represent all possible traffic that with a few lines of specification, and
use the verification tool to verify that the output of all of such traffic is satisfies the query.

### Okay! That's typical formal methodsy stuff! What is 🧛‍♀️'s novelty?

Efficient verification of schedulers with arbitrary large buffer sizes.

The main novelty of 🧛‍♀️modeling is that it allows analysis when considering
buffer sizes of arbitrary large sizes.
The previous tool for performance verification, only capable of very small and unrealistic
buffer sizes (≤1K).

## Experiments

### FPerf

FPerf uses a certain grammar to specify workloads.
Using a CEGIS style search through the state space of the possible
workloads allowed by the grammar, FPerf finds a Workload that
always results in the Query.

FPerf works by using a search loop, inside the loop FPerf generates
a new random workload, and then verifies whether workload always
result in the query.
To verify most of the workloads generated during the search, FPerf calls
a verification engine.

We show that using 🧛‍♀️gives significantly better verification time
than the scheduler model used in FPerf when we increase the buffer size.
We showed that, size of buffer has negligible effect on the verification
time in 🧛‍♀️while FPerf verification time increases by increasing the buffer
size and becomes intractable for larger and realistic buffer sizes.

### How the experiments work?

In each test-case, FPerf generates a set of workloads and verifies each using
Z3.
We record those workloads and verify them in 🧛‍♀️, simulating the situation where
we keep the FPerf's search algorithm, while replacing the formal model of network
as designed in 🧛‍♀️.

We compare the average verification time of individual calls to the verification engine
for each buffer size.

## Getting Started

### Requirements

Install [Docker](https://docs.docker.com/get-started/get-docker/)

### Clone

```shell
git clone https://github.com/all-things-networking/count-buffy.git
cd count-buffy
git submodule update --init --recursive
cp .env.example .env
```

### Pull 🧛‍♀️ Image

```shell
docker compose pull
```

### Hello World Example

The `examples` directory includes a simple example of using 🧛‍♀️ to
model a rate-limiter scheduler and verify certain workload and queries.

```shell
docker compose run --rm buffy hello-world
```

The output shows a sequence of input and output traffic.
Each sequence shows the number of packets at each time step.
In the output traffic, there should be at most one packet
within each two subsequent time steps.

## Experiments 

### Summary of Experiments

| Experiment id | Description                            |
|---------------|----------------------------------------|
| rr            | Round-Robin Scheduler                  |
| prio          | Strict-Priority Scheduler              |
| fq            | FqCoDel Scheduler                      |
| loom          | Compositional                          |

### Run Experiments

For performance evaluation of 🧛‍♀️ we used the 4 case studies used
in FPerf.
For each case study, we run the FPerf search for various buffers sizes
and recorded all workloads generated during the search.
Then we checked these workloads in 🧛‍♀️ to ensure we get the
same SAT/UNSAT result and compare the time of each call to the verification engine.
This way we can compare the efficiency of 🧛‍♀️ against FPerf.

Data for our experimental results include:

- Workloads generated by FPerf
- Output logs of 🧛‍♀️ that includes verification time of each workload
- plots to compare the average verification time of workloads in 🧛‍♀️ and FPerf

To reproduce the result claimed in the paper, we provide two steps:

1. Using a pre-existing set of workloads, verify them in both FPerf and 🧛‍♀️and draw plots comparing the times.
2. Generate the workloads the proceed to the step 1.

The main reason for breaking the experiments reproduction this way is that generating workloads from FPerf is much
more time consuming.
As a result, as a part of this repository, we provide a set of pre-recorded workloads.
However, we also provide the instructions and scripts for the step 2, so the workloads can also be re-generated.

### Compare Verification Time of FPerf and 🧛‍♀️

For this experiment, we are using a set of pre-recorded workloads generated during FPerf search stored in
`data/sub_wls`.
The directory includes sub-directories for each experiment.
For each case study we have separate workloads file for different buffer sizes.
These workloads files include an entry for each workload that FPerf generates and verify with Z3.
The following is an example a workload entry.
The header line (starting with `###`) includes the verification time of FPerf (in ms) and verification result (
SAT/UNSAT).
Following lines specify the workload's constraints.

```text
### - Time: 45 Res: SAT
[1, 6]: cenq(3, t) < 2t
[7, 13]: SUM_[q in {0, 1, 2, 3, 4, }] cenq(q ,t) < 7t
```

The steps are as follows:
1- Verify each workload file in FPerf
2- Verify each workload file in 🧛‍♀️ and record the verification time
3- Draw plots comparing the average verification of time in FPerf vs 🧛‍♀️

#### 1- Verify Workloads in FPerf

The following scripts verifies all workload files for all case studies
in FPerf:

```shell
docker compose run --rm buffy fperf_verify_all_workloads.sh
```

#### 2- Verify Workloads in 🧛‍♀️

Now, we verify all workloads in 🧛‍♀️.
We feed a workload file into 🧛‍♀️ and output is a log file that includes
a line per workload specifying the verification time and the SAT/UNSAT result:

```text
scheduler, buf_size, wl_idx, time_millis, solver_res
prio,10, 0, 27, SAT
prio,10, 1, 33, SAT
prio,10, 2, 9, SAT
```

The log files of 🧛‍♀️ are saved into the `data/logs` directory.

To start the experiment, first we clear the existing 🧛‍♀️ log files:

```shell
rm -rf data/logs data/plots
```

Next, we verify all workloads in the `data/sub_logs` in 🧛‍♀️:

```shell
docker compose run --rm buffy buffy_verify_all_workloads.sh
```

After a successful execution, 🧛‍♀️ log files are saved into the `data/logs` directory.

#### 3- Draw plots of average verification times

After the previous step, we have all the required data to compare the verification time of FPerf and 🧛‍♀️:

- `data/logs` includes the output of 🧛‍♀️
- `data/sub_wls` includes the output of FPerf (pre-recorded)

Now, we can use the following command to draw plots for all case studies:

```shell
docker compose run --rm buffy draw_all_plots.sh
```

plots are saved in the `data/plots` directory.

### Generate Workloads in FPerf

We previously executed FPerf search for each of the case studies and included the recorded workload
files in the `data/sub_wls`.
To generate a fresh version these files, we need to execute the FPerf search.

Use the following command to generate the workloads:

```shell
docker compose run --rm buffy fperf_search_all.sh
```

After a successful execution, workloads generated during the search are saved into the 
`data/new_wls`.

Now, we can execute the previous steps again to compare the verification times using the
newly generate workloads.

> We set the `BUFFY_WLS_DIR=data/new_wls` to point the directory containing the new
> version of the generated workloads


Verify newly generated workloads in Buffy:
```shell
docker compose run --rm buffy fperf_verify_all_workloads.sh
```

> We can skip this step since we just generated the workloads using FPerf itself.

Verify newly generated workloads in 🧛‍♀️:

```shell
docker compose run --rm -e BUFFY_WLS_DIR=data/new_wls buffy buffy_verify_all_workloads.sh
```

draw the plots again:

```shell
docker compose run --rm -e BUFFY_WLS_DIR=data/new_wls buffy draw_all_plots.sh
```

## Walkthrough of the Prio Case-Study

To explain how the implementation works and explain the code we walk through 
the implementation of the `prio` case study.

The entry point is the `prio.cpp` file.
The program accepts two arguments: buffer size and a flag specifying whether
use window constraints or not.

To implement the strict priority scheduler, we derive the sub-class from `STSChecker`.
An `STSChecker` includes the model of the scheduler as well as methods for verifying
combinations of workloads and queries.

We then use `StsRunner` to read workloads of the case study for a single buffer size,
verify each workload, and save the verification times in a log file.

### `STSChecker`

This class includes the scheduler model, base workload and query for a specific case
study. 
Scheduler is implemented as a transition system described below.

#### Scheduler Logic

To implement scheduler behavior, we need to override three methods of the `STSChecker`: `init`, `trs` and `out`.
Each of these methods, return a set of constraints to be added to solver.

##### Initial State:

```cpp
vector<NamedExp> init(ev const &b0, ev const &s0)
```
This method specifies the initial state of the scheduler.
`b0` is a vector of boolean expressions, one for each input 
buffer of the scheduler.
Each `b0[i]` specifies whether the `i`th input buffer is backlogged at time zero.
`s0` is a vector of numeric expressions, representing arbitrary state variables
of the scheduler at time zero.
For instance, in the round-robin scheduler, we use the state variable to record 
the index of the last dequeued buffer, so we can find the buffer that should 
dequeue next.
For the strict priority, we don't need state variables.

The following method shows the `init` method of the priority scheduler.
Basically, here we are allowing any initial state, so we are returning a 
single `true` constraint.
```cpp
vector<NamedExp> PrioSTS::init(const ev &b0, const ev &s0) {
  return {slv.ctx.bool_val(true)};
}
```

#### Transition Relation:

This method defines the transition relation, i.e., the relation between
each pair of subsequent states.

Similar to init, it receives the backlog and state variables vectors. 
However, it receives two pair of such variables, one for timestep `t` and 
another pair for timestep `t+1`.

The method should then return constraints that relate these variables between 
timestep `t` and `t+1`.

```cpp
vector<NamedExp> PrioSTS::trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) {
```

The following method shows the transition relation of the `prio` scheduler:
```cpp
vector<NamedExp> PrioSTS::trs(ev const &b, ev const &s, ev const &bp, ev const &sp, int tp) {
    vector<NamedExp> rv;
    expr res = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        for (int l = i + 1; l < num_bufs; ++l) {
            res = res && (implies(b[i], implies(b[l], bp[l])));
            rv.emplace_back(res);
        }
    }
    return rv;
}
```
Here we are enforcing the constraint `b[i] => (b[l] => bp[l])` for all `l > i`.
Assuming that lower indices have higher priority, basically here we are 
enforcing that if a lower priority buffer is backlogged, then no lower
priority buffer can dequeue a packet in current time step (`b[l] => bp[l]`). 


#### Output Constraints:

The final method for a complete implementation of the priority scheduler, is
the `out` method:
```cpp
vector<NamedExp> PrioSTS::out(const ev &bv, const ev &sv, const ev2 &ov, int t) 
```
The `out` method includes constraints for specifying how the scheduler's output
(packets to be dequeued) should be calculated based on the current state of the
scheduler.
So, it receives the vector of backlogs `bv`, vector of state variables `sv` and
the vector of output packets `ov` for timestep `t`. 
`out` should return constraints that relate `bv` and `sv` with `ov`.

The following method shows the `out` method of the priority scheduler:
```cpp
vector<NamedExp> PrioSTS::out(const ev &bv, const ev &sv, const ev2 &ov, int t) {
    vector<NamedExp> rv;
    expr res = slv.ctx.bool_val(true);
    expr not_until = slv.ctx.bool_val(true);
    for (int i = 0; i < num_bufs; ++i) {
        res = res && ite(not_until && bv[i], ov[i] == 1, ov[i] == 0);
        not_until = not_until && (!bv[i]);
        rv.emplace_back(res);
    }
    return rv;
}
```

Simply, we start from the first buffer, and dequeue a packet from the first 
non-empty buffer.

These methods together define the logic of the scheduler, independent of how
the underlying buffers are modelled.

The constraints for modelling the buffer behavior are implemented in the `STSChecker`,
and shared by all derived schedulers.

Finally, to complete the case study, we need to implement the base workload and query methods.

Base workload returns a set of constraints over the inputs to the buffers
and query returns a set of constraints over the outputs from the buffers.

The case study for the priority scheduler doesn't include a base workload,
so here we only explain the query.

#### Query
The following method shows the query method of the priority scheduler.
The query in the case study specifies whether there exists a timestep
`t` where the third input buffer is blocked for more than five timesteps.

```cpp

vector<NamedExp> PrioSTS::query() {
    expr res = slv.ctx.bool_val(false);
    for (int i = 0; i < timesteps - QUERY_TRESH + 1; ++i) {
        expr part = slv.ctx.bool_val(true);
        for (int j = 0; j < QUERY_TRESH; ++j) {
            part = part && B[2][i + j];
            part = part && (O[2][i + j] == 0);
        }
        res = res || part;
    }
    return {res};
}
```

To know whether a buffer is blocked we check if it is backlogged and 
it doesn't output any packets.

#### `StsRunner` class

This class is responsible for reading the individual workloads from file,
and translating them into constraints over the input buffers.

It then uses the Z3 solver, to verify the model of the scheduler alongside the 
base workload, query and the workload generated by FPerf. 
We also read the expected verification result from the FPerf's output, and
raise an exception if our result is different from FPerf.

