Note: Please see "Project_Description.pdf" for further details. 

Markus Li

Command Used : "/usr/bin/time -p ./wordcount custom_eval [threads] [buffer size] perf_eval.txt perf_out.txt
Format       : @TestX: [threads], [buffer size], [minimum real time out of 5 trials] 

Custom Evaluation Results: 

@Test1: 1, 1000, 6.15
@Test2: 2, 1000, 8.30
@Test3: 4, 1000, 15.79
@Test4: 8, 1000, 24.69
@Test5: 16, 1000, 28.01
@Test6: 32, 1000, 28.27
@Test7: 8, 100, 24.64
@Test8: 8, 1000, 24.61
@Test9: 8, 10000, 24.57

Post Evaluation Thoughts: 

After having gathered and analyzed the results from the custom evaluation, my initial thought was that there may be something lacking in my implementation since as the number of threads increased (with a fixed buffer size), the minimum real time taken out of five trials increased concurrently. However, I came to a realization that the number of threads does not necessarily translate to an overall faster execution of a program since multiple threads each have their own independent stack, pointers, and registers. This means that even though a higher number of threads may execute a specific task faster, time is constantly being stolen between threads (e.g. context switching, waiting) leading to a higher overall, real time taken. Conclusively, regardless of whether a program demands CPU, RAM, or network resources, the resources will always be finite and thus, having additional threads will always be more time consuming if for example, waiting is unconditionally required. 

As for the tests with a fixed thread count and an increasing buffer size (tests 7-9), what I observed was not far from what I expected. The general trend showed a decrease in real-time consumed as the buffer size increased. As threads share memory, the larger the buffer space, the less the occurrences where threads need to compete for time that occurs during context switching or waiting for example. As such, a larger buffer size admits to threads more data simultaneously. 


