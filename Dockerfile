FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y python3 python3-pip g++ cmake pipx && \
    pipx install conan

ENV PATH="/root/.local/bin:$PATH"

WORKDIR /buffy

COPY conanfile.txt .
COPY Makefile .

RUN make init

RUN make install-deps

RUN pip install --no-cache-dir --break-system-packages matplotlib numpy pandas seaborn

COPY src src
COPY examples examples
COPY CMakeLists.txt *.cpp ./

RUN make build

ENV BUFFY_WLS_DIR="data/wls"
ENV BUFFY_LOGS_DIR="data/logs"
ENV PATH="/buffy/build/Release/bin:$PATH"

COPY fperf /fperf

WORKDIR /fperf

RUN make

ENV PATH="/fperf/build/Release/bin:$PATH"

WORKDIR /buffy

COPY scripts scripts

ENV PATH="/buffy/scripts:$PATH"

CMD ["tail", "-f", "/dev/null"]