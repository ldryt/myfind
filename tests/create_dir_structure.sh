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
mkdir dir2

touch file1.txt
touch file2.log
touch .hidden_file
touch "file with spaces.txt"

create_file_with_perms "executable.sh" 755
create_file_with_perms "a" 444
create_file_with_perms "b" 666
create_file_with_perms "c" 323
create_file_with_perms "d" 643
create_file_with_perms "e" 423

ln -s file1.txt symlink_to_file1
ln -s dir1 symlink_to_dir1
ln file2.log hardlink_to_file2

mkfifo my_fifo

touch "file[1].txt"
touch "file(2).txt"
touch "file{3}.txt"
touch "file#4.txt"
touch "file\$5.txt"
touch "file"
touch "fie2"
touch "feww"
touch "few"
touch "fe1"

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

mkdir foo
touch foo/ba{z,r}
ln -s foo qux

echo "Test environment setup complete."
echo "Directory structure:"
tree
