export RESOURCE=$PWD/app/src/main/assets/resource.bfs

# clean resource
rm $RESOURCE

echo RESOURCES
cd resource

# shaders
cd shaders
glslangValidator -V shader.vert -o vert.spv
glslangValidator -V shader.frag -o frag.spv
cd ..

# add resources
bfs $RESOURCE blobSet readme.txt
bfs $RESOURCE blobSet icons/ic_info_outline_white_24dp.png
bfs $RESOURCE blobSet shaders/vert.spv
bfs $RESOURCE blobSet shaders/frag.spv
bfs $RESOURCE blobSet textures/lava.png

# cleanup shaders
rm shaders/*.spv
cd ..

# VKUI
cd app/src/main/cpp/libvkk/vkui/resource
./build-resource.sh $RESOURCE
cd ../../../../../../..

echo CONTENTS
bfs $RESOURCE blobList
