#!/bin/bash

glslc skybox.frag -o skybox.frag.spv
glslc skybox.vert -o skybox.vert.spv
glslc point_light.frag -o point_light.frag.spv
glslc point_light.vert -o point_light.vert.spv
glslc simple_shader.frag -o simple_shader.frag.spv
glslc simple_shader.vert -o simple_shader.vert.spv
glslc shadowmap.frag -o shadowmap.frag.spv
glslc shadowmap.vert -o shadowmap.vert.spv
