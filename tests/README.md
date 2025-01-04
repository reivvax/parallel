# Concurrent Programming C Assignment 2 tests and timer

## About
This package contains two scripts: 
- **test_correctness.sh**: checks correctness of ***nonrecursive*** and ***parallel***
- **timer.sh**: performs some analysis of ***parallel*** execution time compared to ***reference***

## Usage
1. Ensure that `./tests` folder is located in the project main directory and your build directory is named `build` like so:

    ```text
        .
        ├── build
        ├── commmon
        ├── nonrecursive
        ├── parallel
        ├── reference
        └── tests
            ...
            ├── test_correctness.sh
            └── timer.sh
    ```

2. Navigate to `./tests` .

3. Ensure that both: `test_correctness.sh` and `timer.sh` have executable permissions:

    ```bash
        chmod +x ./test_correctness.sh
        chmod +x ./timer.sh
    ```

## test_correctness.sh 
Launch `test_correctness.sh` to test correctness of output and lack of memory leaks of ***nonrecursive*** and ***parallel***. Programs are tested using `prlimit` and `valgrind`.  
The script compares just the first line of output, namely the total sum of multiset, as there may exist more than one correct solutions.
```sh
    ./test_correctness.sh
```
In case of any errors, all the files needed for analysis will be in `.tests/correctness_output` directory.

## timer.sh
To analyze execution time of ***parallel*** simply launch `timer.sh`.  
```sh
    ./timer.sh
```

The script in default configuration runs ***parallel*** for parameters:  

$d \in \{5, 10, 15, 20, 25, 30, 32, 34\}$ and $t \in \{1, 2, 4, 8, 16, 32\}$  

It **does not** run ***reference*** to make testing faster. It takes the times from predefined list (in `reference_times.py`). Those times are the mean times measured on ***students*** machine with ***reference*** built with `CMAKE_BUILD_TYPE=Release` (so with `-O3`).  

The output for every test case is:
```text
    Reference real time
    Parallel real time
    Parallelization facator
    CPU utilization
```
Where:
$$parallelization\_factor = \frac{reference\_real\_time}{parallel\_real\_time}$$

$$cpu\_utilization = \frac{parallel\_user\_time}{parallel\_real\_time * HELPER\_THREADS\_COUNT}$$

**If any of measured times using `time` was equal to 0.00, then this time is treated as 0.001, for computation convenience.**

Note user time, not real time in numerator.  
If CPU utilization is **"NEGLIGIBLE"**, it means that parallelization_real_time or parallelization_user_time was measured to be 0.00, so computing this CPU utilization in such case would not give much information anyway.

You can change some behaviour of the script using options:

### Options:

* -b | --brief  
When this option is set, the output instead of default verbose format, is in CSV format, which then is easy to work with using pandas. Probably the most useful option.  
Easy to pipeline with python script that will generate plots.
The columns in output are:  
***d, t, reference_real_time, parallel_real_time, parallelization_factor***

* -r | --reference  
When this option is set, the script also runs ***reference*** to compare it's time with ***parallel***.  
Useful if you don't test on ***students***.

* -t | --threads_limit \<n\>  
When this option is set followed by positive integer `n`, then it limits the maximal number of used threads to $2^n$.  
For example:
    ```sh
        ./timer.sh -t 4
    ```
    Will limit the `t` parameter to values $\{1, 2, 4, 8, 16\}$.  
    Using `-t 7` will run ***parallel*** with `t` parameter equal to all powers of two up to 128.  
    In the same time `-t 7` is the maximal value accepted by the script.

* -d | -d_limit \<n\>  
This option works very similar to `-t`. It limits the maximal value of `d` parameter.  
Instead of powers of two, parameter for this option denotes the index of maximal used `d` from list $\{5, 10, 15, 20, 25, 30, 32, 34\}$ (indexing from 1).  
For example:
    ```sh
        ./timer.sh -d 3
    ```
    Will limit `d` parameter to values $\{5, 10, 15\}$.

**Of course options can be combined:**
```sh
    ./timer.sh -r -t 7 -b
```