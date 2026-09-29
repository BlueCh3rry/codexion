*This project has been created as part of the 42 curriculum by mmakhmae.*

## Description

Codexion is a multithreaded simulation inspired by the classic "Dining Philosophers"
problem, reframed around a team of software coders sharing a limited pool of
dongles needed to compile their code. Each coder repeatedly needs to
acquire the two dongles adjacent to them, compile, then release the dongles, debug,
and refactor — for a configured number of cycles.

The goal of the project is to correctly manage concurrent access to shared resources, while:
- avoiding deadlocks
- avoiding starvation
- detecting when a coder has been "burned out"
- producing an accurate, race-free log of every coder's state changes.

Two scheduling are supported:
- **FIFO** (First–in First-out) coders compile in order they requested.
- **EDF** (Earliest Deadline First) a coder with the closest deadline compile first.

## Instructions

### Compilation

```bash
make          # builds the `codexion` binary
```

Other available targets:

```bash
make clean    # remove object files
make fclean   # remove object files and the binary
make re       # fclean + full rebuild
make run      # run the project
make leak     # run the project with valgrind option
make helgrind # run the project with with helgrind tool
```

### Execution

```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> <number_of_compiles_required> \
           <dongle_cooldown> <scheduler>
```

| `number_of_coders` | Number of coder threads (must be > 1) |

| `time_to_burnout` | Time in ms before a coder stuck compiling is declared "burned out" |

| `time_to_compile` | Time in ms a compile cycle takes |

| `time_to_debug` | Time in ms a debug cycle takes |

| `time_to_refactor` | Time in ms a refactor cycle takes |

| `number_of_compiles_required` | Number of compile cycles each coder must complete |

| `dongle_cooldown` | Time in ms a dongle stays unavailable after being released |

| `scheduler` | Scheduling strategy: `fifo` or `edf` |

```bash
./codexion 5 800 200 100 100 5 100 fifo
```

## Blocking cases handled

Deadlock prevention — Coders acquire their two dongles in a fixed global order based on dongle ID. This prevents circular-wait deadlocks. \
Scheduling / starvation control — FIFO uses increasing request tickets, while EDF uses coder deadlines with the coder ID as a tie-breaker. A coder must be at the top of both dongle heaps before compiling. \
Cooldown handling — After a compile, both dongles receive an available_at timestamp. A coder cannot compile while either required dongle is still in its cooldown period. \
Simulation shutdown — sim_sleep() regularly checks done, allowing running coders to stop when the simulation ends. Waiting coders are notified through cond_thread. \
Log serialization — State changes are sent through log_state(), which uses the logging mutex to prevent concurrent output from being mixed.

## Thread synchronization mechanisms

pthread_mutex_t mutex per dongle — Protects each dongle so that only one coder can use it at a time. \
state_mutex — Protects shared simulation and scheduling state such as done, tickets, deadlines, compile timestamps, cooldown timestamps, and compile counters. \
pthread_cond_t cond_thread — Coordinates coder threads waiting for scheduling priority or resource availability and wakes them when the state changes. \
Request heaps — Each dongle maintains a priority heap containing waiting coders. FIFO tickets or EDF deadlines determine which request has priority. \
log_mutex — Serializes logging so messages from different threads remain readable and consistent.

## Resources

- Documentation & Guides
- Official 42 subject pdf
- https://www.geeksforgeeks.org/sorting-algorithms/
- POSIX Threads Programming man pages

AI was used during this project to identify which parts were genuinely necessary and for debugging/fixing errors.
