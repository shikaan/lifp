FROM debian

WORKDIR /mnt

RUN apt update && apt install -y make gpg lsb-release wget

RUN wget https://apt.llvm.org/llvm.sh &&\
    chmod +x llvm.sh &&\
    ./llvm.sh 21 &&\
    apt install -y clang-21

RUN ln -s $(which clang-21) /usr/bin/clang

CMD ["bash"]
