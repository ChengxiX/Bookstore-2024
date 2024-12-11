import os
import random

def generate_commands(num_commands):
    commands = []
    inserted_pairs = []  # 存储已插入的 (index, value) 对
    segment_size = 2000  # 每个阶段的命令数量，可根据需要调整
    for i in range(0, num_commands, segment_size):
        # 定义阶段性的命令概率
        if (i // segment_size) % 2 == 0:
            # 偶数阶段，insert命令概率高
            command_types = ['insert', 'delete', 'find']
            command_probs = [0.75, 0.15, 0.10]
        else:
            # 奇数阶段，delete命令概率高
            command_types = ['insert', 'delete', 'find']
            command_probs = [0.15, 0.7, 0.15]
        # 在当前阶段生成命令
        for _ in range(min(segment_size, num_commands - i)):
            cmd_type = random.choices(command_types, weights=command_probs)[0]
            if cmd_type == 'insert':
                index = random.randint(1, 30)
                value = random.randint(1, 2147483646)
                commands.append(f'insert {index} {value}')
                inserted_pairs.append((index, value))
            elif cmd_type == 'delete':
                if inserted_pairs:
                    index, value = random.choice(inserted_pairs)
                    commands.append(f'delete {index} {value}')
                    # 可选：从已插入的列表中移除已删除的项
                    inserted_pairs.remove((index, value))
                else:
                    # 当没有已插入的项时，跳过或生成其他命令
                    index = random.randint(1, 30)
                    value = random.randint(1, 2147483646)
                    commands.append(f'insert {index} {value}')
                    inserted_pairs.append((index, value))
            else:  # cmd_type == 'find'
                if inserted_pairs and random.random() < 0.95:
                    # 95%的概率使用已插入的索引
                    index = random.choice([pair[0] for pair in inserted_pairs])
                else:
                    index = random.randint(1, 1000)
                commands.append(f'{cmd_type} {index}')
    return commands

def generate_test_files(num_files, num_commands_per_file, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    for i in range(num_files):
        file_path = os.path.join(output_dir, f'test_input_{i+1}.txt')
        with open(file_path, 'w') as file:
            file.write(f'{num_commands_per_file}\n')
            commands = generate_commands(num_commands_per_file)
            file.write('\n'.join(commands))

if __name__ == '__main__':
    # 要生成的文件数量
    num_files = 3
    # 每个文件的命令数量
    num_commands_per_file = 10000
    # 输出目录
    output_dir = 'test_inputs'
    generate_test_files(num_files, num_commands_per_file, output_dir)
