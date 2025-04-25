
words = set()
with open("shakespeare_raw.txt", "r") as f:
    for line in f.readlines():
        words.add(line.strip() + "\n")

with open("shakespeare.txt", "w") as f:
    f.writelines(words)        