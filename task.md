Run this md file without head -n until we endorse it or the philosopher dies.
If it dies when you run the should live test case, modify the code so that it survives.
There is a flaw in the algorithm when the number of philosophers is n%2 == 1, and philosophers who should have priority to fork do not fork. Improved so that once a philosopher is in THINK, he/she has the highest priority to get a meal.

- should live
    - `./philo 5 800 200 200`
    - `./philo 5 800 200 200 7`
    - `./philo 4 410 200 200`
- should die
    - `./philo 1 800 200 200`
    - `./philo 4 310 200 100`