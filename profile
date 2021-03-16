# Update SDK to point to the Android SDK
SDK=$HOME/Android/Sdk
export SDK

# Setup Vulkan for Linux SDL
if [[ "$OSTYPE" != "darwin"* ]]; then
	source ${HOME}/vulkan/1.2.170.0/setup-env.sh
fi

#-- DON'T CHANGE BELOW LINE --

export ANDROID_SDK_ROOT=$SDK
export PATH=$SDK/tools:$PATH
export PATH=$SDK/platform-tools:$PATH
export PATH=$SDK/platform-tools/systrace:$PATH
export PATH=$SDK/build-tools/27.0.3:$PATH
export PATH=$SDK/build-tools/28.0.3:$PATH
export TOP=`pwd`
alias croot='cd $TOP'
