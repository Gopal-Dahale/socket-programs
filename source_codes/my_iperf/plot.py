import matplotlib.pyplot as plt
import numpy as np

f_throughputs = open('throughputs.txt')
f_avg_delays = open('avg_delays.txt')

# Read throughputs
throughputs = []
for line in f_throughputs:
    throughputs.append(float(line))

# Read avg_delays
avg_delays = []
for line in f_avg_delays:
    avg_delays.append(float(line))


# Plot throughputs vs time
plt.plot(throughputs, label='Throughput')
plt.xlabel('Time (s)')
plt.ylabel('Throughput (Bytes/sec)')
plt.title('Throughput vs Time')
plt.legend()
plt.show()

# Plot avg_delays vs time
plt.plot(avg_delays, label='Average Delay')
plt.xlabel('Time (s)')
plt.ylabel('Average Delay (µs)')
plt.title('Average Delay vs Time')
plt.legend()
plt.show()