if arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 "$2" 
then
   arduino-cli upload --port "$1"  --fqbn esp8266:esp8266:nodemcuv2 "$2"
fi
