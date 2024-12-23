import hashlib
import os

def hash_file(file_path, buffer_size=65536):
    """Calculates the hash of a file given its path."""
    hasher = hashlib.md5()
    with open(file_path, "rb") as file:
        buf = file.read(buffer_size)
        while buf:
            hasher.update(buf)
            buf = file.read(buffer_size)
    return hasher.hexdigest()

def compare_files(first_file, second_file):
    """Checks if two files are equal by comparing their hashes."""
    return hash_file(first_file) == hash_file(second_file)

def analyze_directories(client, server):
    """Analyzes two directories, looking for files with the same name and checking if they are equal."""
    files_client = {file: os.path.join(client, file) for file in os.listdir(client) if os.path.isfile(os.path.join(client, file))}
    files_server = {file: os.path.join(server, file) for file in os.listdir(server) if os.path.isfile(os.path.join(server, file))}

    common_files = set(files_client.keys()).intersection(files_server.keys())

    for file in common_files:
        first_file = files_client[file]
        second_file = files_server[file]

        if compare_files(first_file, second_file):
            print(f"The files \"{file}\" inside \"{client}\" and \"{server}\" are the same.")
        else:
            print(f"The files \"{file}\" inside \"{client}\" and \"{server}\" are the same name but different content.")

    unique_in_client = set(files_client.keys()) - common_files
    unique_in_server = set(files_server.keys()) - common_files

    if unique_in_client:
        print(f"\nThe unique files in \"{client}\" are:\n{"\n".join(unique_in_client)}")
    if unique_in_server:
        print(f"\nThe unique files in \"{server}\" are:\n{"\n".join(unique_in_server)}")

if __name__ == "__main__":
    client = "../client-files/"
    server = "../server-files/"
    analyze_directories(client, server)
