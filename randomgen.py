import random

file = open("randnums.txt", "a")
nums = list(range(25, 1000001))
random.shuffle(nums)
for i in nums:
    file.write(str(i) + "\n")
