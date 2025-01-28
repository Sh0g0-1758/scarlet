import os
import sys

def count_lines_in_file(file_path):
    """Count the number of lines in a single file."""
    try:
        with open(file_path, 'r') as file:
            return sum(1 for line in file)
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
        return 0

def count_lines_in_directory(directory_path):
    """Count the number of lines in all files within a directory."""
    total_lines = 0
    for root, _, files in os.walk(directory_path):
        for file_name in files:
            file_path = os.path.join(root, file_name)
            total_lines += count_lines_in_file(file_path)
    return total_lines

def main():
    if len(sys.argv) < 2:
        print("Usage: python line_counter.py <directory_or_file> ...")
        sys.exit(1)

    total_lines = 0

    for path in sys.argv[1:]:
        if os.path.isfile(path):
            total_lines += count_lines_in_file(path)
        elif os.path.isdir(path):
            total_lines += count_lines_in_directory(path)
        else:
            print(f"Invalid path: {path}")

    print(f"Total number of lines in all files: {total_lines}")

if __name__ == "__main__":
    main()
