# Update SDK to point to the Android SDK
SDK=$HOME/Android/Sdk

# Vulkan SDK for SDL
source ${HOME}/vulkan/1.1.121.1/setup-env.sh

#-- DON'T CHANGE BELOW LINE --

export PATH=$SDK/tools:$PATH
export PATH=$SDK/platform-tools:$PATH
export PATH=$SDK/platform-tools/systrace:$PATH
export PATH=$SDK/build-tools/27.0.3:$PATH
export PATH=$SDK/build-tools/28.0.3:$PATH
export TOP=`pwd`
alias croot='cd $TOP'
