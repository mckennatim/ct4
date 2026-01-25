#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "connWIFI.h"

bool setupWIFI() {
    WiFiManager wm;

    // Optional: Add debug output
    // wm.setDebugOutput(true);

    // Optional: Set a timeout (in seconds) so it doesn't hang forever in AP mode
    // if configuration fails. If it times out, it returns false.
    wm.setConfigPortalTimeout(180); // 3 minutes to configure

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name
    // "AutoConnectAP", password "password" is optional
    bool res = wm.autoConnect("CT_Monitor_Setup"); // AP Name

    if(!res) {
        Serial.println("Failed to connect or hit timeout");
        return false;
    } 
    
    Serial.println("WiFi Connected... yeey :)");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void resetWIFI() {
    WiFiManager wm;
    wm.resetSettings();
    Serial.println("WiFi settings reset");
}