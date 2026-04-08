#!/bin/bash

shaderfiles=()
while IFS= read -r line; do
  if [[ "${line:0:8}" != "compiled" ]]; then
    shaderfiles+=("$line")
  fi
done < <(rg --files)

IFS='/'
count=0
for val in "${shaderfiles[@]}"; do

  [[ $val == "shadercompile.sh" ]] && continue

  read -ra isolated <<<"$val"
  comp="compiled/"${isolated[1]}".spv"

  if [ ! -f "$comp" ] || [[ $(date -r "$val" +%s) > $(date -r "$comp" +%s) ]]; then
    glslc "$val" -o "$comp"
    echo "compiled "$val""
  fi

done
