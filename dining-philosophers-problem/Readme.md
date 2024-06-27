# Dining Philosophers Problem

## Table of Contents
- [Problem Overview](#problem-overview)
- [Approach](#approach)
- [Implementation Details](#implementation-details)
- [Example Output](#example-output)

## Problem Overview

5 philosophers sit in a circular table with 5 chairs. They will alternate between 2 states, think and eat. There are 5 single chopsticks on each sides of the philosophers. A philosopher can only eat if they can pick up 2 chopsticks.

The goal is to implement in a deadlock free way and avoid starvation

## Approach

- **States**: Each philosopher can be in one of two states: thinking or eating.
- **Avoid Deadlock**: Philosophers only pick up chopsticks if both are available. They wait their turn based on the queue.
- **Avoid Starvation**: The use of a queue ensures that every philosopher will eventually get their turn to eat.
- **Critical Sections**: The critical section of this problem is the access of the chopsticks array and the queue for eating order.

## Implementation Details

### Classes

- `Table`: Manages the state of chopsticks and philosophers. Contains mutexes and condition variables for synchronization.
- `ThreadParam`: Holds parameters for each philosopher thread.

### Functions

- `lineUpToEat`: Adds a philosopher to the queue when they are hungry.
- `think`: Simulates the philosopher thinking.
- `pickUpChopsticks`: Philosopher tries to pick up chopsticks when they are hungry.
- `putDownChopsticks`: Philosopher puts down chopsticks after eating.
- `eat`: Simulates the philosopher eating.
- `philosopher`: The main function for philosopher threads, alternating between thinking and eating.

### Usage

Compile the program using `g++`:

g++ -pthread -o dining_philosophers dining_philosophers.cpp

Run the program:

./dining_philosophers <total_running_time_seconds> <time_eating_microseconds> <time_thinking_microseconds>

#### Example:

./dining_philosophers 10 1000000 2000000

This will run the simulation for 10 seconds, with each philosopher eating for 1 second and thinking for 2 seconds.

## Example Output

Philosopher [0] thinking!
Philosopher [1] thinking!
Philosopher [2] thinking!
Philosopher [3] thinking!
Philosopher [4] thinking!
Philosopher [0] picks up the chopsticks: left = [0], right = [1]
Philosopher [0] eating!
Philosopher [0] puts down the chopsticks: left = [0], right = [1]
Philosopher [1] picks up the chopsticks: left = [1], right = [2]
Philosopher [1] eating!
Philosopher [1] puts down the chopsticks: left = [1], right = [2]
...
Philosopher [0] has ate [25234] times!
Philosopher [1] has ate [25357] times!
Philosopher [2] has ate [25248] times!
Philosopher [3] has ate [25331] times!
Philosopher [4] has ate [25325] times!
