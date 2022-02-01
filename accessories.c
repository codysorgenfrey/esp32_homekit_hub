#include "common.h"
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
	;/* Identify heat pump here */
}

// WeMo switch characteristics
homekit_characteristic_t switchOn = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t switchName = HOMEKIT_CHARACTERISTIC_(NAME, "WeMo Switch");

// SimpliSafe characteristics
homekit_characteristic_t ssCurState = HOMEKIT_CHARACTERISTIC_(SECURITY_SYSTEM_CURRENT_STATE, 0);
homekit_characteristic_t ssTarState = HOMEKIT_CHARACTERISTIC_(SECURITY_SYSTEM_TARGET_STATE, 0);
homekit_characteristic_t ssName = HOMEKIT_CHARACTERISTIC_(NAME, "SimpliSafe");

// Garage Door characteristics
homekit_characteristic_t gdCurState = HOMEKIT_CHARACTERISTIC_(CURRENT_DOOR_STATE, 0);
homekit_characteristic_t gdTarState = HOMEKIT_CHARACTERISTIC_(TARGET_DOOR_STATE, 0);
homekit_characteristic_t gdObstruction = HOMEKIT_CHARACTERISTIC_(OBSTRUCTION_DETECTED, false);
homekit_characteristic_t gdName = HOMEKIT_CHARACTERISTIC_(NAME, "Garage Door Opener");


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
            &switchOn,
            &switchName,           
            NULL
        }),
        NULL
    }),
    HOMEKIT_ACCESSORY(.id=3, .category=homekit_accessory_category_security_system, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
			HOMEKIT_CHARACTERISTIC(NAME, "SimpliSafe"),
			HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
			NULL
		}),
        HOMEKIT_SERVICE(SECURITY_SYSTEM, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &ssCurState,
            &ssTarState,
            &ssName,         
            NULL
        }),
        NULL
    }),
    HOMEKIT_ACCESSORY(.id=4, .category=homekit_accessory_category_garage, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
			HOMEKIT_CHARACTERISTIC(NAME, "Garage Door Opener"),
			HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
			NULL
		}),
        HOMEKIT_SERVICE(GARAGE_DOOR_OPENER, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &gdCurState,
            &gdTarState,
            &gdObstruction,
            &gdName,         
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