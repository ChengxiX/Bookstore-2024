import os
import random

def generate_commands(num_commands):
    commands = []
    inserted_indices = []
    for _ in range(num_commands):
        cmd_type = random.choice(['insert', 'delete', 'find'])
        if cmd_type in ['insert', 'delete']:
            index = random.randint(1, 30)
            value = random.randint(1, 100000)
            commands.append(f'{cmd_type} {index} {value}')
            if cmd_type == 'insert':
                inserted_indices.append(index)
        else:  # cmd_type == 'find'
            if inserted_indices and random.random() < 0.8:
                # 80%的概率使用已插入的索引
                index = random.choice(inserted_indices)
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
    num_commands_per_file = 1000
    # 输出目录
    output_dir = 'test_inputs'
    generate_test_files(num_files, num_commands_per_file, output_dir)
