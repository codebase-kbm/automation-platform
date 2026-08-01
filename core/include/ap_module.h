#ifndef AP_MODULE_H
#define AP_MODULE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
	/* Modules-List */

	#define AP_MODULE_LIST(X) \
		X(AP_MODULE_SYSTEM,  "system") \
		X(AP_MODULE_TIMEOUT, "timeout") \
		X(AP_MODULE_SOCKET,  "socket") \
		X(AP_MODULE_MQTT,    "mqtt") \
		X(AP_MODULE_CAN,     "can")

	typedef enum
	{
		AP_MODULE_NONE = 0,

	#define AP_MODULE_ENUM(name, string) name,
		AP_MODULE_LIST(AP_MODULE_ENUM)
	#undef AP_MODULE_ENUM

		AP_MODULE_COUNT

	} ap_module_type_t;

const char *ap_module_type_name(
    ap_module_type_t type);

ap_module_type_t ap_module_type_from_name(
    const char *name);

#define AP_MODULE_TEST 99

#ifdef __cplusplus
}
#endif

#endif /* AP_MODULE_H */