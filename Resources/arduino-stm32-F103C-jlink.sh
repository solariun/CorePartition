#arduino-cli compile -u  -p "$1" -b Arduino_STM32:STM32F1:genericSTM32F103C:upload_method=jlinkMethod,opt=osstd "$2" 
arduino-cli compile -u  -p "$1" -b Arduino_STM32:STM32F1:genericSTM32F103C:upload_method=jlinkMethod,opt=o3std "$2" 

