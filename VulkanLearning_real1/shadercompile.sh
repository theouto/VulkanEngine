#!/bin/bash

glslc_catch()
{
  echo "missing glslc!" && exit
}

rg_catch()
{
  echo "you should install rg (ripgrep)" && exit
}

compilation_error()
{
  echo "unable to compile ""$val"
  echo "ignore the message below telling you that it did compile, it's lying."
}

trap 'glslc_catch' ERR

glslc --help >/dev/null

trap 'rg_catch' ERR

cd ./shaders/

[ ! -d ./compiled ] && mkdir ./compiled

shaderfiles=()
while IFS= read -r line; do
  if [[ "${line:0:8}" != "compiled" ]]; then
    shaderfiles+=("$line")
  fi
done < <(rg --files)

trap 'compilation_error' ERR

IFS='/'
for val in "${shaderfiles[@]}"; do

  read -ra isolated <<<"$val"
  comp="compiled/"${isolated[1]}".spv"

  if [ ! -f "$comp" ] || [[ $(date -r "$val" +%s) > $(date -r "$comp" +%s) ]]; then
    glslc "$val" -o "$comp"
    echo "compiled "$val""
  fi

done

cd ..
