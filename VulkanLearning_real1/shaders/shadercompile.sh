#!/bin/bash

glslc main_pass/skybox.frag -o compiled/skybox.frag.spv
glslc main_pass/skybox.vert -o compiled/skybox.vert.spv
glslc main_pass/point_light.frag -o compiled/point_light.frag.spv
glslc main_pass/point_light.vert -o compiled/point_light.vert.spv
glslc main_pass/simple_shader.frag -o compiled/simple_shader.frag.spv
glslc main_pass/simple_shader.vert -o compiled/simple_shader.vert.spv
glslc lighting/shadowmap.frag -o compiled/shadowmap.frag.spv
glslc lighting/shadowmap.vert -o compiled/shadowmap.vert.spv
