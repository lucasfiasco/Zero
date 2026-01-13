# Zero
run:
    cmake --build build
    ./build/zero src/test.zero
    nasm -felf64 out.asm
    ld out.o && ./a.out
