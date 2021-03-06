#ifndef DOMOTIC_PI_DEFINE
#define DOMOTIC_PI_DEFINE

#define STR_(X)
#define STR(X) STR_(X)

// Enables mutex lock in setter functions
#define DOMOTIC_PI_THREAD_SAFE

#define DOMOTIC_PI_APPLE_HOMEKIT

#ifdef DOMOTIC_PI_APPLE_HOMEKIT

#ifndef DOMOTIC_PI_THREAD_SAFE
#define DOMOTIC_PI_THREAD_SAFE
#endif

#define DOMOTIC_PI_APPLE_HOMEKIT_MANUFACTURER "Flow3r"

#endif // DOMOTIC_PI_APPLE_HOMEKIT


// Highest valid pin number
#ifndef DOMOTIC_PI_MAX_PIN
#define DOMOTIC_PI_MAX_PIN 63
#endif

// Standard pin mode and pull up/down state to reset pins after usage
#ifndef DOMOTIC_PI_PIN_STANDARD_MODE
#define DOMOTIC_PI_PIN_STANDARD_MODE INPUT
#endif
#ifndef DOMOTIC_PI_PIN_STANDARD_PUD
#define DOMOTIC_PI_PIN_STANDARD_PUD PUD_DOWN
#endif

#define DOMOTIC_PI_JSON_INPUT "Input.json"
#define DOMOTIC_PI_JSON_OUTPUT "Output.json"
#define DOMOTIC_PI_JSON_COMM "Comm.json"
#define DOMOTIC_PI_JSON_PROGRAMMED_EVENT "ProgrammedEvent.json"
#define DOMOTIC_PI_JSON_DOMOTIC_NODE "DomoticNode.json"

#include <memory>

namespace domotic_pi {

	class IInput;
	typedef std::shared_ptr<IInput> Input_ptr;

	class IOutput;
	typedef std::shared_ptr<IOutput> Output_ptr;

	class IComm;
	typedef std::shared_ptr<IComm> Comm_ptr;

	class DomoticNode;
	typedef std::shared_ptr<DomoticNode> DomoticNode_ptr;
}

#endif