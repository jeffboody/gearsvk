export RESOURCE=$PWD/app/src/main/assets/resource.pak

echo RESOURCES
cd resource

# shaders
glslangValidator -V shader.vert -o vert.spv
glslangValidator -V shader.frag -o frag.spv
pak -c $RESOURCE vert.spv
pak -a $RESOURCE frag.spv
pak -a $RESOURCE lava.png
rm *.spv

# icons
pak -a $RESOURCE ic_arrow_back_white_24dp.texz
pak -a $RESOURCE ic_info_outline_white_24dp.texz
cd ..

# VKUI
cd app/src/main/cpp/libvkk/vkui/resource
./build-resource.sh $RESOURCE
cd ../../../../../../..

echo CONTENTS
pak -l $RESOURCE
