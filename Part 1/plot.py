import subprocess
import os
import json
import numpy as np
import matplotlib.pyplot as plt

# Files
stats_file = "stats.txt"
config_file = "config.json"

with open(config_file, 'r') as f:
    config = json.load(f)
config['num_clients'] = 1
config['p'] = 1

with open(config_file, 'w') as f:
    json.dump(config, f, indent=4)


average_list = []

# Create the stats.txt file (this will also clear it if it already exists)
print(f"Creating {stats_file}...")
with open(stats_file, 'w') as file:
    pass  # This just creates/clears the file

for iteration in range(10):
    print(f"running iteration for words per packet = {iteration + 1}")

    # values = []
    
    for i in range(10):
        result = subprocess.run(["make", "run"], capture_output=True, text=True)
        
    with open(config_file, 'r') as f:
        config = json.load(f)
    config['num_clients'] = 1
    config['p'] = config['p'] + 1 

    with open(config_file, 'w') as f:
        json.dump(config, f, indent=4)

    print(f"Config updated successfully for iteration {iteration + 1}!\n")


file_path = 'stats.txt'


with open(file_path, 'r') as file:
    data = file.read().strip()
    values = data.split() 

print(len(values))

for i in range(10):
    l = 0
    cnt = 0
    for j in range(10):
        if(i*10 + j >= len(values)):
            break
        l = l + float(values[10*i + j])
        cnt+=1
    
    if(cnt == 0):
        average_list.append(0)
    else:
        average_list.append((l/cnt)*1000)
    
# After 10 iterations, plot the average values
x = []
for i in range(len(average_list)):
    x.append(i+1)

plt.plot(x, average_list, marker='o', linestyle='-', color='b', label='Average download Time for 1 client')
plt.xlabel('Words per packet')
plt.ylabel('Average Download Time(ms)')
plt.title('Average Download Time vs Words per packet')
plt.legend()
plt.grid(True)
# plt.show()
plt.savefig('average_completion_time.png')  # Save the plot to a file


# Optionally print or save the final average list
print("Average List:", average_list)
