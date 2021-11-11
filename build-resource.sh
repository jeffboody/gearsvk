export RESOURCE=$PWD/app/src/main/assets/resource.pak

echo RESOURCES
cd resource

# shaders
cd shaders
glslangValidator -V draw.vert -o draw_vert.spv
glslangValidator -V draw.frag -o draw_frag.spv
glslangValidator -V scaler.vert -o scaler_vert.spv
glslangValidator -V scaler.frag -o scaler_frag.spv
cd ..

# pak resources
pak -c $RESOURCE readme.txt
pak -a $RESOURCE icons/ic_arrow_back_white_24dp.texz
pak -a $RESOURCE icons/ic_info_outline_white_24dp.texz
pak -a $RESOURCE shaders/draw_vert.spv
pak -a $RESOURCE shaders/draw_frag.spv
pak -a $RESOURCE shaders/scaler_vert.spv
pak -a $RESOURCE shaders/scaler_frag.spv
pak -a $RESOURCE textures/lava.png

# cleanup shaders
rm shaders/*.spv
cd ..

# VKUI
cd app/src/main/cpp/libvkk/vkui/resource
./build-resource.sh $RESOURCE
cd ../../../../../../..

echo CONTENTS
pak -l $RESOURCE
