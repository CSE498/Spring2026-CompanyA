# Curtis
 - (Example Script info)

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