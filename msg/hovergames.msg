# HOVERGAMES_STATUS message data

uint64 timestamp		# time since system start (microseconds)

uint8 STANDBY_STATE   = 0
uint8 PREACTIVE_STATE = 1
uint8 ACTIVE_STATE    = 2
uint8 FAULTED_STATE   = 3

uint8 RECORDING_STATE     = 0
uint8 IMAGE_CAPTURE_STATE = 1
uint8 PAUSED_STATE        = 2

uint8 hovergames_sm
uint8 hovergames_activesm

#TOPICS hovergames


