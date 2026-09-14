*This project has been created as part of the 42 curriculum by mmakhmae.*

# Codexion

## Description

Codexion is a multithreaded simulation inspired by the classic "Dining Philosophers"
problem, reframed around a team of software coders sharing a limited pool of
dongles needed to compile their code. Each coder repeatedly needs to
acquire the two dongles adjacent to them, compile, then release the dongles, debug,
and refactor — for a configured number of cycles.

The goal of the project is to correctly manage concurrent access to shared resources, while:
- avoiding deadlocks,
- avoiding starvation,
- detecting when a coder has been "burned out" (stuck compiling for too long),
- producing an accurate, race-free log of every coder's state changes.

Two scheduling strategies are supported:
- **FIFO** — (First–in First-out) coders compile strictly in turn order.
- **EDF** (Earliest Deadline First–inspired) — a coder may compile as soon as both
  of its dongles are available again (post-cooldown), independent of turn order.

## Instructions

### Compilation

```bash
make
```

This builds the `codexion` binary using the project `Makefile`.

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

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coder threads (must be > 1) |
| `time_to_burnout` | Time in ms before a coder stuck compiling is declared "burned out" |
| `time_to_compile` | Time in ms a compile cycle takes |
| `time_to_debug` | Time in ms a debug cycle takes |
| `time_to_refactor` | Time in ms a refactor cycle takes |
| `number_of_compiles_required` | Number of compile cycles each coder must complete |
| `dongle_cooldown` | Time in ms a dongle stays unavailable after being released |
| `scheduler` | Scheduling strategy: `fifo` or `edf` |

Example:

```bash
./codexion 5 800 200 100 100 5 100 fifo
```

## Blocking cases handled

- **Deadlock prevention** — Each coder needs its left and
  right dongle simultaneously (hold-and-wait). To break the circular-wait condition
  that causes classic Dining Philosophers deadlocks, dongles are always locked in a
  fixed global order: the coder compares `left->id` and `right->id` and always locks
  the lower-id dongle first. This guarantees no cycle of threads can each be holding
  one dongle while waiting on the next, which removes the possibility of a deadlock.
- **Starvation prevention** — In FIFO mode, coders compile strictly in id order via
  a shared `order` counter, so every coder is guaranteed a turn. In EDF mode,
  availability is tracked with per-dongle timestamps (`available_at`), so a coder is
  never blocked indefinitely by another coder holding resources longer than the
  cooldown allows.
- **Cooldown handling** — After a compile, both dongles are stamped with
  `available_at = now + dongle_cooldown` before being unlocked. `coder_can_compile()`
  checks these timestamps so a coder cannot immediately snatch a dongle again before
  its cooldown has elapsed, preventing any single coder from monopolizing resources.
- **Precise burnout detection** — A dedicated monitor thread (`coder_chrono`) polls
  every coder's `last_compile_start` timestamp under `state_mutex`. If a coder has
  been compiling longer than `time_to_burnout`, the simulation is stopped
  (`done = 1`) and a `pthread_cond_broadcast` wakes every thread currently waiting on
  the condition variable, so no thread is left blocked forever once the simulation
  ends.
- **Log serialization** — All state-change logs go through `log_state()`, which is
  protected by a dedicated `log_mutex`. This guarantees log lines from different
  coder threads are never interleaved or corrupted, and that timestamps are printed
  in a consistent, race-free order.

## Thread synchronization mechanisms

- **`pthread_mutex_t` per dongle** — Each dongle has its own mutex. A coder must
  lock both its adjacent dongles' mutexes before compiling, and unlocks them right
  after. This is the core mechanism preventing two coders from using the same
  dongle at the same time.
- **`state_mutex`** — A single mutex protecting all shared simulation state (`done`,
  `order`, each coder's `last_compile_start`, each dongle's `available_at`). Any
  thread reading or writing this shared state — coder threads, the monitor thread,
  or `main` during cleanup — must hold `state_mutex` first. This is what prevents
  race conditions such as two threads reading/writing `data->order` or `data->done`
  at the same time.
- **`pthread_cond_t cond_thread`** — Used as the coordination signal between coder
  threads and the scheduler:
  - In FIFO mode, a coder that is not next in `order` calls `pthread_cond_wait()`
    and sleeps until it is woken by `ft_signal()` (called by the coder who just
    finished compiling), which advances `order` and broadcasts.
  - In EDF mode, a coder waits on the same condition variable until
    `coder_can_compile()` becomes true; every completed compile triggers a
    `pthread_cond_broadcast()` so all waiting coders re-check their availability.
  - The monitor thread also broadcasts on this condition variable when it detects
    burnout, guaranteeing no coder thread is left permanently blocked in
    `pthread_cond_wait()` once the simulation is flagged as done.
- **`log_mutex`** — A dedicated mutex solely for serializing calls to
  `log_state()`, decoupled from `state_mutex` so that logging doesn't block
  simulation logic (dongle acquisition, compile timing) and vice versa.

Together, these primitives ensure that: dongles are never used by two coders at
once (mutual exclusion), the simulation state is always read/written consistently
(state_mutex), coder threads are woken up correctly instead of busy-waiting
(cond_thread), and the console output remains readable and race-free (log_mutex).

## Resources

- Documentation & Guides
- Official 42 subject (push_swap)
- https://www.geeksforgeeks.org/sorting-algorithms/
- POSIX Threads Programming (`pthread_mutex_*`, `pthread_cond_*`) — man pages
  (`man pthread_mutex_lock`, `man pthread_cond_wait`, etc.)
- Coffman's conditions for deadlock

### AI usage

AI (Claude, by Anthropic) was used during this project for:
- Reviewing commented-out code to identify which parts were genuinely necessary
- Drafting and structuring this README.md file.
