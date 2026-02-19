# Curtis
- Overview of expression parser class and expected function of class:
    - A class that creates an object which takes in an expression and returns a function object which evaluates the expression given a container (map<string, double>).
- Explain implementation:
    - CollectVariables() uses lexer to tokenize the expression and collect all variables (ignoring duplicates) in a first seen order.
    - Parser validates the input expression syntax by using te_compile with fake variables before returning the object
    - Returns a function (lambda) which takes in the container and evaluates the original expression.
        - searches the map for the variable first to ensure it exists
        - creates bindings pointing to the values
        - compiles the expression, evaluates, frees the ast that tinyexpr creates, and returns the result.
- Potential use case(s):
    - For shop/trading system in interactive world to compute skill points
    - By other groups for data processing between world/agent relations


# Caleb

This class is a Scheduler, which is responsible for choosing which process should run next based on a priority heuristic.

Each process has an ID and a priority value, and processes with higher priority are more likely to be selected.

There are helper functions called FindProcess that allow the scheduler to locate a process by ID, with both const and non-const versions to preserve const correctness.

Internally, the scheduler stores a list of processes, where each process consists of an ID and a priority value.  It also owns a random number generator, which is used to implement weighted random selection.

The AddProcess function inserts a new process with a given priority, ensuring the priority is positive and the ID is unique.

The heart of the scheduler is the Next() function.
This function implements weighted random scheduling, also known as lottery scheduling.

First, it computes the total priority by summing the priority of every process.
Then it generates a random number between zero and that total. Each process occupies a portion of that range proportional to its priority.

As the scheduler walks through the process list, it accumulates priorities until the cumulative value exceeds the random number. The process that crosses that threshold is selected.

This approach ensures that higher-priority processes run more frequently, while still allowing lower-priority processes to be scheduled, preventing starvation.

HasProcess checks whether a process exists.

RemoveProcess deletes a process if it exists and returns whether the removal was successful.

UpdatePriority allows priorities to change dynamically, which is useful for AI or simulation systems where importance changes over time. It does this by first checking if a process exists, then updating its priority value.

Clear removes all processes from the scheduler, and Empty checks if the scheduler is empty.


# EventQueue

Hi, my name is Ho Wang Ho, or Howard. I am responsible for implementing the EventQueue class in our Interactive World module.

The EventQueue class is designed to store and manage scheduled events in the game world. Each event contains a time value, a unique ID, an insertion order, and a payload function that will be executed later. The queue always returns the event with the smallest time first. If two events have the same time, the one that was inserted earlier will be processed first.

The ScheduleEvent function adds a new event into the queue and automatically assigns it a unique ID and insertion order. The PeekNextEvent function allows us to see which event will happen next without removing it from the queue. The PopNextEvent function removes and returns the next event in order.
The CancelEvent function uses a technique called lazy cancellation. Instead of removing the event immediately from the priority queue, it marks the event as cancelled in a separate set. When a cancelled event reaches the top of the queue, it will be skipped automatically. This keeps the implementation simple and efficient.

Finally, the Empty function checks whether there are any remaining valid events in the queue.
Overall, this class focuses on ordering and managing events efficiently, and it can be used to implement features like cooldown timers, delayed actions, or scheduled world updates in our game system.
