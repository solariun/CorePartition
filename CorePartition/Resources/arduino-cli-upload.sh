if arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 WiFiDotMatrixDisply
then
   arduino-cli upload --port /dev/cu.wchusbserial1461140  --fqbn esp8266:esp8266:nodemcuv2 WiFiDotMatrixDisply
fi
