#include "common.h"
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
	;/* Identify heat pump here */
}

// WeMo switch characteristics
homekit_characteristic_t wemo_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t wemo_name = HOMEKIT_CHARACTERISTIC_(NAME, "WeMo Switch");


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_bridge, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, HK_UNIQUE_NAME),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, HK_MANUFACTURER),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, HK_SERIALNUM),
            HOMEKIT_CHARACTERISTIC(MODEL, HK_MODEL),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, HK_SKETCH_VER),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            NULL
        }),
        NULL
    }), 
    HOMEKIT_ACCESSORY(.id=2, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
			HOMEKIT_CHARACTERISTIC(NAME, "WeMo Switch"),
			HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
			NULL
		}),
        HOMEKIT_SERVICE(SWITCH, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &wemo_on,
            &wemo_name,           
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
		.accessories = accessories,
		.password = HK_PASSWORD
};