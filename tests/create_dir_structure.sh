#!/usr/bin/env bash

set -e

test "$#" -ne 1 && echo "Need path of directory as argument" && exit 1

cd "$1"

create_file_with_perms() {
    filename=$1
    perms=$2
    touch "$filename"
    chmod "$perms" "$filename"
}

mkdir -p dir1/dir2/dir3
mkdir -p dirA/dirB
mkdir -p .hidden_dir
mkdir "dir with spaces"

touch file1.txt
touch file2.log
touch .hidden_file
touch "file with spaces.txt"

create_file_with_perms "executable.sh" 755
create_file_with_perms "readonly.txt" 444

ln -s file1.txt symlink_to_file1
ln -s dir1 symlink_to_dir1
ln file2.log hardlink_to_file2

mkfifo my_fifo

touch "file[1].txt"
touch "file(2).txt"
touch "file{3}.txt"
touch "file#4.txt"
touch "file\$5.txt"

touch dir1/1.txt
touch dir1/1.txta
touch dir1/1.txtaa
touch dir1/dir2/2.txt
touch dir1/dir2/dir3/3.txt
touch dir1/dir2/dir3/3.txta
touch dir1/dir2/dir3/xt
touch file.jpeg
touch file.png
touch file.tar.gz
touch file.zip

create_file_with_perms "suid_file" 4755
create_file_with_perms "sgid_file" 2755
create_file_with_perms "sticky_file" 1755

echo "Test environment setup complete."
echo "Directory structure:"
tree
