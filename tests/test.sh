#!/usr/bin/env bash

test "$#" -ne 1 && echo "Need path of myfind as argument" && exit 1

myfind_exec=$(realpath "$1")

tmpdir=$(mktemp -d)
./tests/create_dir_structure.sh "$tmpdir"
cd "$tmpdir"

test_find() {
    params="$@"
    echo "Testing with parameters: $params"

    find_output=$(find $params 2>/dev/null)

    myfind_output=$("$myfind_exec" $params 2>/dev/null)

    if diff <(echo "$find_output") <(echo "$myfind_output"); then
        echo "Test passed"
        echo "---"
    else
        echo "------"
        echo "ᕕ(╭ರ╭ ͟ʖ╮•́)⊃¤=(————-"
        echo "Test failed with parameters: $params"
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

echo "All tests passed ٩(^‿^)۶"
