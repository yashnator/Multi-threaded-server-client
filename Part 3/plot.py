import subprocess
import os
import json
import numpy as np
import matplotlib.pyplot as plt
import time

# Files
stats_file = "stats.txt"
config_file = "config.json"

average_list = []


with open(config_file, 'r') as f:
    config = json.load(f)

config['num_clients'] = 1

with open(config_file, 'w') as f:
    # Save the updated configuration back to the JSON file
    json.dump(config, f, indent=4)


# Create the stats.txt file (this will also clear it if it already exists)
print(f"Creating {stats_file}...")
with open(stats_file, 'w') as file:
    pass  # This just creates/clears the file

for clients in range(16):
    print(f"running iteration for no of clients = {max(clients*2, 1)}")
    
    for i in range(3):
    # Run the 'make run' command and capture the output
        result = subprocess.run(["make", "run-beb"], capture_output=True, text=True)
    
    with open(config_file, 'r') as f:
        config = json.load(f)

    config['num_clients'] = max((clients+1)*2, 1)

    with open(config_file, 'w') as f:
        # Save the updated configuration back to the JSON file
        json.dump(config, f, indent=4)
    
time.sleep(2)

file_path = 'stats.txt'

with open(file_path, 'r') as file:
    data = file.read().strip()
    vals_mapped = data.split() 


result = {}

for item in vals_mapped:
    index, value = item.split(":")
    index = int(index)
    value = float(value)
    
    if index in result:
        result[index].append(value)
    else:
        result[index] = [value]

for key, val in result.items():
    lll = 0
    for v in val:
        lll+=v
    lll = lll/len(val)
    result[key] = lll

# print(len(result))

x = result.keys()
average_list = result.values()

plt.plot(x, average_list, marker='o', linestyle='-', color='b', label='Average download Time per client')
plt.xlabel('no of clients')
plt.ylabel('Average Download Time(ms)')
plt.title('Average download Time vs no of clients')
plt.legend()
plt.grid(True)
# plt.show()
plt.savefig('average_completion_time.png')  # Save the plot to a file


