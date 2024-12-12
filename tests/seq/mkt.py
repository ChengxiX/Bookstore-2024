import random

commands = []
answers = []
seq = []

n = 10000  # 操作次数

push_count = 0
pop_count = 0

for _ in range(n):
    ops = ['push', 'size', 'update', 'at', 'range' # , 'resize'
           ]
    # if push_count > pop_count:
        # ops.append('pop')
    op = random.choice(ops)
    if op == 'push':
        x = random.randint(1, 100)
        commands.append(f'push {x}')
        seq.append(x)
        push_count += 1
    elif op == 'pop':
        commands.append('pop')
        if seq:
            seq.pop()
        pop_count += 1
    elif op == 'size':
        commands.append('size')
        answers.append(str(len(seq)))
    elif op == 'resize':
        x = random.randint(0, 100)
        commands.append(f'resize {x}')
        if x < len(seq):
            seq = seq[:x]
        else:
            seq.extend([0] * (x - len(seq)))
    elif op == 'update':
        if seq:
            idx = random.randint(0, len(seq) - 1)
            val = random.randint(1, 100)
            commands.append(f'update {idx} {val}')
            seq[idx] = val
        else:
            continue
    elif op == 'at':
        if seq:
            idx = random.randint(0, len(seq) - 1)
            commands.append(f'at {idx}')
            answers.append(str(seq[idx]))
        else:
            continue
    elif op == 'range':
        if seq:
            l = random.randint(0, len(seq) - 1)
            r = random.randint(l + 1, len(seq))
            commands.append(f'range {l} {r}')
            answers.append(' '.join(map(str, seq[l:r])))
        else:
            continue

with open('1.in', 'w') as f:
    f.write('\n'.join(commands))

with open('1.out', 'w') as f:
    f.write('\n'.join(answers))
