#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

// Nordic UART Service (NUS) UUIDs - widely supported by BLE terminal apps
#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHAR_RX_UUID        "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  // Write (phone → device)
#define CHAR_TX_UUID        "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  // Notify (device → phone)

USBHIDKeyboard Keyboard;
BLECharacteristic *pTxCharacteristic;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// LED pin for AtomS3U (built-in NeoPixel is on GPIO 35, but we'll use simple indication)
#define LED_PIN 35

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("BLE Client Connected");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE Client Disconnected");
    }
};

class KeyboardCallbacks : public BLECharacteristicCallbacks {
    // Trim whitespace from both ends of string
    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = str.find_last_not_of(" \t\r\n");
        return str.substr(start, end - start + 1);
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string raw = pCharacteristic->getValue();

        if (raw.length() > 0) {
            std::string input = trim(raw);

            Serial.print("Received: [");
            Serial.print(input.c_str());
            Serial.println("]");

            // Check for macro chain (|| delimiter)
            if (input.find("||") != std::string::npos) {
                executeMacro(input);
            } else if (input[0] == '!') {
                handleSpecialCommand(input);
            } else {
                // Type the received string as-is (including newlines if present)
                typeText(raw);
            }

            // Send confirmation back to phone
            pTxCharacteristic->setValue("OK");
            pTxCharacteristic->notify();
        }
    }

    void typeText(const std::string& text) {
        for (char c : text) {
            if (c == '\n') {
                Keyboard.press(KEY_RETURN);
                Keyboard.release(KEY_RETURN);
            } else if (c != '\r') {
                Keyboard.print(c);
            }
        }
    }

    void executeMacro(const std::string& macro) {
        std::string remaining = macro;
        std::string delimiter = "||";
        size_t pos;

        while ((pos = remaining.find(delimiter)) != std::string::npos) {
            std::string segment = remaining.substr(0, pos);
            executeSegment(trim(segment));
            remaining = remaining.substr(pos + delimiter.length());
            delay(50);  // Small delay between macro steps
        }
        // Execute last segment
        if (!remaining.empty()) {
            executeSegment(trim(remaining));
        }
    }

    void executeSegment(const std::string& segment) {
        if (segment.empty()) return;

        Serial.print("Macro segment: [");
        Serial.print(segment.c_str());
        Serial.println("]");

        if (segment[0] == '!') {
            handleSpecialCommand(segment);
        } else {
            typeText(segment);
        }
    }

    void handleSpecialCommand(const std::string& cmd) {
        if (cmd == "!ENTER") {
            Keyboard.press(KEY_RETURN);
            Keyboard.release(KEY_RETURN);
        }
        else if (cmd == "!TAB") {
            Keyboard.press(KEY_TAB);
            Keyboard.release(KEY_TAB);
        }
        else if (cmd == "!ESC") {
            Keyboard.press(KEY_ESC);
            Keyboard.release(KEY_ESC);
        }
        else if (cmd == "!BACKSPACE") {
            Keyboard.press(KEY_BACKSPACE);
            Keyboard.release(KEY_BACKSPACE);
        }
        else if (cmd == "!UP") {
            Keyboard.press(KEY_UP_ARROW);
            Keyboard.release(KEY_UP_ARROW);
        }
        else if (cmd == "!DOWN") {
            Keyboard.press(KEY_DOWN_ARROW);
            Keyboard.release(KEY_DOWN_ARROW);
        }
        else if (cmd == "!LEFT") {
            Keyboard.press(KEY_LEFT_ARROW);
            Keyboard.release(KEY_LEFT_ARROW);
        }
        else if (cmd == "!RIGHT") {
            Keyboard.press(KEY_RIGHT_ARROW);
            Keyboard.release(KEY_RIGHT_ARROW);
        }
        // Sticky modifier keys (hold until release or key press)
        else if (cmd == "!HOLD+CTRL") {
            Keyboard.pressRaw(0xE0);  // Left Ctrl
        }
        else if (cmd == "!HOLD+ALT") {
            Keyboard.pressRaw(0xE2);  // Left Alt
        }
        else if (cmd == "!HOLD+SHIFT") {
            Keyboard.pressRaw(0xE1);  // Left Shift
        }
        else if (cmd == "!HOLD+SUPER") {
            Keyboard.pressRaw(0xE3);  // Left GUI
        }
        // Common combos (still supported)
        else if (cmd == "!CTRL+C") {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('c');
            Keyboard.releaseAll();
        }
        else if (cmd == "!CTRL+V") {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('v');
            Keyboard.releaseAll();
        }
        else if (cmd == "!CTRL+Z") {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('z');
            Keyboard.releaseAll();
        }
        else if (cmd == "!CTRL+A") {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('a');
            Keyboard.releaseAll();
        }
        else if (cmd == "!CTRL+T") {
            Keyboard.press(KEY_LEFT_CTRL);
            delay(20);
            Keyboard.press('t');
            delay(20);
            Keyboard.releaseAll();
        }
        else if (cmd == "!CTRL+W") {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('w');
            Keyboard.releaseAll();
        }
        else if (cmd == "!CTRL+S") {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press('s');
            Keyboard.releaseAll();
        }
        else if (cmd == "!ALT+TAB") {
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.press(KEY_TAB);
            Keyboard.releaseAll();
        }
        else if (cmd == "!SUPER") {
            // Use pressRaw with 0xE3 (USB HID Left GUI) to force report send
            Keyboard.pressRaw(0xE3);
            delay(300);
            Keyboard.releaseAll();
        }
        else if (cmd == "!SUPER2") {
            // Try right GUI key
            Keyboard.press(KEY_RIGHT_GUI);
            delay(200);
            Keyboard.releaseAll();
        }
        else if (cmd == "!SUPER+R") {
            // Windows Run dialog
            Keyboard.press(KEY_LEFT_GUI);
            delay(20);
            Keyboard.press('r');
            delay(20);
            Keyboard.releaseAll();
        }
        else if (cmd == "!SUPER+D") {
            // Show desktop
            Keyboard.press(KEY_LEFT_GUI);
            delay(20);
            Keyboard.press('d');
            delay(20);
            Keyboard.releaseAll();
        }
        else if (cmd == "!SUPER+E") {
            // Windows Explorer
            Keyboard.press(KEY_LEFT_GUI);
            delay(20);
            Keyboard.press('e');
            delay(20);
            Keyboard.releaseAll();
        }
        else if (cmd == "!SUPER+A") {
            // GNOME show applications
            Keyboard.press(KEY_LEFT_GUI);
            delay(20);
            Keyboard.press('a');
            delay(20);
            Keyboard.releaseAll();
        }
        else if (cmd == "!SUPER+L") {
            // Lock screen
            Keyboard.press(KEY_LEFT_GUI);
            delay(20);
            Keyboard.press('l');
            delay(20);
            Keyboard.releaseAll();
        }
        else if (cmd.rfind("!F", 0) == 0 && cmd.length() <= 4) {
            // Handle F1-F12
            int fNum = atoi(cmd.substr(2).c_str());
            if (fNum >= 1 && fNum <= 12) {
                uint8_t fKey = KEY_F1 + (fNum - 1);
                Keyboard.press(fKey);
                Keyboard.release(fKey);
            }
        }
        else if (cmd == "!RELEASE") {
            Keyboard.releaseAll();
        }

        Serial.print("Special command: ");
        Serial.println(cmd.c_str());
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("AtomS3U BLE Keyboard Starting...");

    // Initialize USB HID Keyboard
    USB.begin();
    Keyboard.begin();
    Serial.println("USB HID Keyboard initialized");

    // Initialize BLE
    BLEDevice::init("AtomS3U-Keyboard");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // RX characteristic (phone writes here)
    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHAR_RX_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pRxCharacteristic->setCallbacks(new KeyboardCallbacks());

    // TX characteristic (device notifies phone)
    pTxCharacteristic = pService->createCharacteristic(
        CHAR_TX_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pTxCharacteristic->addDescriptor(new BLE2902());

    // Start service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("BLE Keyboard ready! Device name: AtomS3U-Keyboard");
    Serial.println("Using Nordic UART Service (NUS) - compatible with BLE terminal apps");
}

void loop() {
    // Handle BLE reconnection
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        BLEDevice::startAdvertising();
        Serial.println("Restarting BLE advertising...");
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }

    delay(10);
}
