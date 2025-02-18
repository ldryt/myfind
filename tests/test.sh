#!/usr/bin/env bash

test "$#" -ne 1 && echo "Need path of myfind as argument" && exit 1

myfind_exec=$(realpath "$1")

tmpdir=$(mktemp -d)
./tests/create_dir_structure.sh "$tmpdir"
cd "$tmpdir"

test_find() {
    echo "Testing with parameters: $@"

    find_output=$(find "$@")
    find_status=$(echo $?)

    myfind_output=$("$myfind_exec" "$@")
    myfind_status=$(echo $?)

    if diff <(echo "$find_output") <(echo "$myfind_output") && test $myfind_status -eq $find_status; then
        echo "Test passed"
        echo "---"
    else
        echo "------"
        echo "$tmpdir"
        echo "ᕕ(╭ರ╭ ͟ʖ╮•́)⊃¤=(————-"
        echo "Test failed with parameters: $@"
        echo "find_status: $find_status"
        echo "myfind_status: $myfind_status"
        exit 1
    fi
}

test_find
test_find *
test_find ???
test_find * -name '*'
test_find * -name '????' -o -name '???'
test_find * -name '????' -o -name '???'
test_find * -name '????' -o -name '???' -a -name 'dir1'
test_find * ! -name '*'
test_find * ! -name '????' -o -name '???'
test_find * -name '????' -o ! -name '???'
test_find * -name '????' -o ! -name '???' -a -name 'dir1'
test_find * ! -name '????' -o -name '???' -a -name 'dir1'

test_find \( -name '*1*' -o \( -name '*.*' -a -name '*t*' \) \)
test_find ! \( -name '*1*' -o \( -name '*.*' -a -name '*t*' \) \)
test_find ! \( -name '*1*' -o \( -name '*.*' -a ! -name '*t*' \) \)

test_find . -name "*.txt"

test_find . -type f
test_find . -type ff
test_find . -type d
test_find . -type l
test_find . -type p

test_find . -perm 644
test_find . -perm 755
test_find . -perm /5
test_find . -perm /1
test_find . -perm /2
test_find . -perm -644
test_find . -perm -755
test_find . -perm -3

test_find . -newer dir2
test_find dir2 -newer dir1
test_find dir1 -newer dir2
test_find dir2 -newer dir2

test_find * -user ldryt
test_find * -group users

test_find qux
test_find -L .
test_find -H qux

test_find * ! -name '????' -o -name '???' -a -name 'dir1' -print
test_find * ! -name '????' -o -name '???' -print -a -name 'dir1'
test_find * ! -name '????' -o -name '???' -print -a -name 'dir1'
test_find * ! -name '????' -o -name '???' -print -a -name 'dir1' -print
test_find * ! -name '????' -o -print -name '???' -print -a -name 'dir1' -print

test_find idonotexist
test_find -newer idonotexist

test_find . -type f -exec echo File: {} \;
test_find . -type f -exec echo File:{} \;
test_find . -type d -exec echo Directory: {} \;
test_find . -type d -exec echo Directory:{} \;
test_find . -type d -exec echo Directory: {}\;
test_find . -type l -exec echo Symlink: {} \;
test_find . -type f -exec cat {} \;
test_find . -type f -exec ls -l {} \;
test_find . -name "*.txt" -exec echo Processing {} \;
test_find . -perm 644 -exec chmod 600 {} \; -print
test_find . -perm 755 -exec chmod 700 {} \; -print

test_find . -exec echo {} {}w{}{}}{{} \;
test_find . -exec echo {} {}w{{}}{} \;
test_find . -exec echo {} }w}{}}{{} \;

test_find . -exec echo {} \; -exec echo Again: {} \;
test_find . -exec echo First: {} \; -exec echo Second: {} \;
test_find . -type f -exec echo File: {} \; -o -type d -exec echo Dir: {} \;

echo "All tests passed ٩(^‿^)۶"
