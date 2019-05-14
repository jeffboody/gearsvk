#!/bin/bash

glslangValidator -V shader.vert -o ../app/src/main/assets/shaders/vert.spv
glslangValidator -V shader.frag -o ../app/src/main/assets/shaders/frag.spv
