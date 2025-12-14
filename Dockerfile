FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y python3 python3-pip g++ cmake pipx && \
    pipx install conan

ENV PATH="/root/.local/bin:$PATH"

WORKDIR /buffy

COPY conanfile.txt .

RUN conan profile detect --force

RUN conan install . --build=missing -s compiler.cppstd=20

#COPY . .

ENTRYPOINT /bin/bash