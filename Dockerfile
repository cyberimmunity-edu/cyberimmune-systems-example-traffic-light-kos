FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive
ENV PATH="${PATH}:/opt/KasperskyOS-Community-Edition-1.2.0.89/toolchain/bin"

COPY KasperskyOS-Community-Edition-1.2.0.89_en.deb /kos/

RUN apt-get update && apt-get install -y --fix-broken /kos/KasperskyOS-Community-Edition-1.2.0.89_en.deb curl mc && adduser --disabled-password --gecos "" user


CMD ["bash"]
