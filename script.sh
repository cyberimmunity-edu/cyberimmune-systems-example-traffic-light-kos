#!/bin/bash

SCRIPT_DIR=$(cd "$(dirname ${0})"; pwd)

BUILD=$(pwd)/build
mkdir -p $BUILD && cd $BUILD