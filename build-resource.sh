cd resource

# build shaders
glslangValidator -V shader.vert -o vert.spv
glslangValidator -V shader.frag -o frag.spv
glslangValidator -V gen_shader.vert -o gen_vert.spv
glslangValidator -V gen_shader.frag -o gen_frag.spv

# pak resources
./pak -c ../app/src/main/assets/resource.pak vert.spv
./pak -a ../app/src/main/assets/resource.pak frag.spv
./pak -a ../app/src/main/assets/resource.pak gen_vert.spv
./pak -a ../app/src/main/assets/resource.pak gen_frag.spv
./pak -a ../app/src/main/assets/resource.pak lava.png
./pak -l ../app/src/main/assets/resource.pak
