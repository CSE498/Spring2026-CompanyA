ActionLog:

Hi, this is Daniel and I'll be going over the ActionLog class for Company A:

HPP:

So in the HPP, we have a structure called Action, which holds an AgentID, a string representing the type of action like movement or attacking, a double representing the current timestamp, some doubles representing position and new position, and a double showing the sequence number or turn.

We have another structure called ActionLog which is just a vector that holds actions, alongside the current time.

We also have some basic features, such as functions to log actions and update the current time, alongside some getters to get actions, get actions within a range of time, get all actions from a specific agent, getting the total number of actions, and a clearing function.

CPP:

And here is the specific implementation for such. For logging, it takes all the data and puts it into an action, and then pushes it onto the ActionLog vector. UpdateTime works as a standard setter, and GetAction and GetActionCount works as a standard getter. For GetActionRange and GetAgentActions, both of them loop through the vector and return the data if it matches up with what the user wants, and Clear works as a standard clearing function.

Tests:

For testing, we have some basic testing of asserting the action count being zero when initialized, maintaining the correct size when logging actions, and then testing many of the functions.


DataLog:
Implementation By: Aneesh Joshi

Goal:
The goal of the data log class is to keep track of numerical data over a span of time. It allows
users to compute statistical metrics such as the count, min, max, mean, and median of all the
data values collected up until that point.
Why it’s useful:
This class will be useful to our company for various different reasons. For our company we plan
on creating a dungeon crawler style game so certain metrics that we could track over time may
include player health, enemies defeated, or damage dealt for example. Based on those findings,
the datalog class can compute stats to provide feedback on metrics in game. It will also provide
a way to see if the game is performing as expected or if it needs any improvements.
How it was implemented and used:
When a data log object is created, it records a start timestamp using std::chrono::steady_clock.
Every time the add function is called, the class stores the value inputted along with how many
seconds have passed since the data log was created.
The class then returns a reference to the collection of samples which can be used to better
inspect the data.The Datalog class can clear and count the samples present.
By using these data samples, the class can then calculate the mean, median, min, max, and
count of the data values collected at that time. They all handle the case where the data log may
be empty using std::optional.
Example usage:
Here is an example use of the data log class, it shows how users can add numerical data to the
class and compute these metrics on the stored data.




ReplayDriver:

I am Jack Rouse, and the class I created is called ReplayDriver. Currently, it's missing its full implementation because the visual aspects of the code have not been added yet. For now, this class acts as a way to view what happened throughout the course of the game. It has multiple options for listing out all the actions of the game through different functions. The class takes an actionLog either at creation or through setActionLog(), which can also reset it and create a new actionLog. To save an action log call SaveReplayToFile after the ReplayLog that you want. IsActionLog checks if an action log is assigned.  The next functions all take an actionLog and save it in different ways. Replay will save the entire action log. Replaybystep allows you to replay by a certain number of steps, like every 3 actions, to get a shorter overall view of the log. 
Replay by time range allows you to replay an action over a certain time throughout the game, like 5-10 seconds or 300-400 seconds of the game. Just give it a start and endtime in seconds.
Replay by agent works by inputting an agent id and will save all actions from that agent. 
Replay by action type allows you to input the action type string and see when all of those actions occurred. In the future, this class will hopefully be able to replay the full game visually to watch back old runs.



Timer:
The Timer class serves as a tool to hold multiple different timings, allowing for easy stopwatch
functionality as well as comparison of durations. The constructor for a Timer object does not
require any parameters, and the default constructor is the only one implemented. Once you’ve
created a timer, call the start function, passing in the name of your timing instance, to create and
start that timer. As the underlying data structure for the timers is an unordered map, make sure
to use different names for each timing instance. If you want to stop your timer, simply call the
stop function with the name of your timer and it will pause the timer. Calling start on a stopped
timer will start the timer back up again, while keeping the duration that the timer had run for
previously. If you want to reset the accumulated time on a timer while keeping the same name,
you can call the restart function, which will set the duration of the timer to zero while starting it
back up again. If you want to get the time which a specific timer has run for, you can use the
getTime function. This function returns the accumulated time of a specific timer in seconds,
returning a value of 0 if the specified timer does not exist in the map of timers yet. The
accumulated duration of a timer is stored in nanoseconds and converted to seconds only when
getTime is called, so when calling getTime you will get a very precise representation of how long
your timer has run for. The last function of note is the isRunning function, which very simply
returns true if the specified timer exists within the map of timers and is currently running,
returning false otherwise.



Tyler Murray
=============================
OutputManager - Class Overview
=============================

GENERAL PURPOSE
---------------
OutputManager is a centralized, thread-safe logging utility that formats
categorized messages with optional timestamps, filters them by severity,
and dispatches them to configurable output sinks such as console,
files, or CSV logs.

=============================
CORE RESPONSIBILITIES
=============================

1. Log Level Management
-----------------------
Manages logging based on severity levels such as:
    DEBUG = 0,
    Verbose = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Silent = 5 (won't log anything)

This allows the system to control how verbose logging should be.


2. Log Categorization
---------------------
Supports categories (e.g., System, UI, Performance, etc.)
to organize logs and make debugging easier.


3. Timestamp Support
-------------------
Optionally has timestamps that show elapsed time
since the program started.


4. Sink Routing
--------------
Routes formatted log messages to registered sinks instead of
hardcoding output to std::cout.

Possible sinks include:
- Console/file output
- CSV logging


5. Thread Safety
----------------
Used a mutex to ensure safe logging in multi-threaded environments.

6. CSV / File Logging
---------------------
Supports optional structured logging to CSV or files for
analysis, debugging, and replay systems.

Test Cases and Output Structure
--------------------------
Has two optional test output cases that will specifically
output to the terminal. these are the csv test case and
timestamp test case.
Here is what the time stamp one will output
[12ms] [Info] [System] Message text

The CSV test case will output a "test_log.csv" into the main
company folder. This will later be moved to its own folder