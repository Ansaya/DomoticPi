#ifndef DOMOTIC_PI_LIB
#define DOMOTIC_PI_LIB

#include "domoticPiDefine.h"
#include "domoticPi.h"

#include "exceptions.h"
#include "OutState.h"

#include "Pin.h"
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include "IAHKAccessory.h"
#endif
#include "IModule.h"
#include "Serializable.h"

#include "InputFactory.h"
#include "IInput.h"
#include "DigitalInput.h"
#include "MqttInput.h"

#include "OutputFactory.h"
#include "IOutput.h"
#include "DigitalOutput.h"
#include "MqttOutput.h"
#include "MqttVolume.h"
#include "SerialOutput.h"

#include "CommFactory.h"
#include "IComm.h"
#include "MqttComm.h"
#include "SerialInterface.h"

#include "DomoticNode.h"

#endif