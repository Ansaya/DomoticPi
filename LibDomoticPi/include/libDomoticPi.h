#ifndef DOMOTIC_PI_LIB
#define DOMOTIC_PI_LIB

#include "domoticPiDefine.h"
#include "domoticPi.h"

#include "exceptions.h"
#include "OutState.h"

#include "Pin.h"
#ifdef DOMOTIC_PI_APPLE_HOMEKIT
#include "HasHAPAccessory.h"
#endif
#include "Module.h"
#include "Serializable.h"

#include "Input.h"
#include "DigitalInput.h"

#include "Output.h"
#include "DigitalOutput.h"
#include "SerialOutput.h"

#include "SerialInterface.h"

#include "DomoticNode.h"

#endif