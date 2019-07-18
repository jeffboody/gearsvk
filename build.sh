#!/bin/bash

# Generate timestamp
echo `date` > app/src/main/assets/timestamp.raw

# Build APK
./gradlew assemble
