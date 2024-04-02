#include <hidapi/hidapi.h>

#include <iomanip>
#include <iostream>
#include <vector>

const uint16_t APPLE_VENDOR_ID = 1452;

const uint16_t SUPPORTED_DISPLAYS[12] = {
    37430, 37415, 37414, 37411, 37410, 37406,
    37404, 37409, 37401, 37400, 37399, 37398,
};

struct hid_report {
    uint8_t report_id;
    uint8_t data[64];
};

class Display {
    hid_device_info *info;
    hid_device *display;

    int currentBrightness = -1;

   public:
    std::wstring name;
    std::wstring manufacturer;

    Display(hid_device_info *info) : info(info) {
        display = hid_open_path(info->path);
        if (display == nullptr) {
            std::cerr << "Failed to open display" << std::endl;
            std::wcerr << hid_error(display) << std::endl;
            return;
        }

        wchar_t data[128] = {};
        if (hid_get_product_string(display, data, sizeof(data)) == -1) {
            std::cerr << "Failed to read product string" << std::endl;
            std::wcerr << hid_error(display) << std::endl;
            return;
        }

        name = data;

        if (hid_get_manufacturer_string(display, data, sizeof(data)) == -1) {
            std::cerr << "Failed to read manufacturer string" << std::endl;
            std::wcerr << hid_error(display) << std::endl;
            return;
        }

        manufacturer = data;

        uint8_t report_descriptor[HID_API_MAX_REPORT_DESCRIPTOR_SIZE] = {};
        int report_descriptor_size = hid_get_report_descriptor(
            display, report_descriptor, HID_API_MAX_REPORT_DESCRIPTOR_SIZE);

        if (report_descriptor_size == -1) {
            std::cerr << "Failed to read report descriptor" << std::endl;
            std::wcerr << hid_error(display) << std::endl;
            return;
        }

        std::cout << "Report descriptor size: " << report_descriptor_size
                  << " bytes" << std::endl;

        for (int i = 0; i < report_descriptor_size; i++) {
            std::cout << "0x" << std::setfill('0') << std::setw(2) << std::hex
                      << (int)report_descriptor[i] << " ";
        }

        std::cout << std::endl;
    }

    ~Display() { hid_close(display); }
};

int main(int argc, char **argv) {
    // Initialize the HIDAPI library
    if (hid_init() != 0) {
        std::cerr << "Failed to initialize HIDAPI" << std::endl;
        return 1;
    }

    // Enumerate all the connected HID devices
    hid_device_info *devices = hid_enumerate(0, 0);
    if (devices == nullptr) {
        std::cerr << "Failed to enumerate HID devices" << std::endl;
        std::wcerr << hid_error(nullptr) << std::endl;
        return 1;
    }

    std::vector<hid_device_info *> displays;

    // Linked list traversal
    for (hid_device_info *device = devices; device != nullptr;
         device = device->next) {
        // Check if the device is a supported display
        bool supported = false;
        for (unsigned short i : SUPPORTED_DISPLAYS) {
            if (device->product_id == i) {
                supported = true;
                break;
            }
        }

        if (supported) {
            displays.push_back(device);
        }
    }

    if (displays.size() == 0) {
        std::cerr << "No supported displays found" << std::endl;
        return 1;
    }

    // Prompt the user to select a display
    std::cout << "Select a display >" << std::endl;

    int count = 0;
    for (auto display : displays) {
        std::wcout << "  " << count++ << ": (" << display->manufacturer_string
                   << ") " << display->product_string << std::endl;
    }

    bool valid = false;
    int selection = -1;

    while (!valid) {
        std::cout << "Selection (0-" << displays.size() - 1 << ") > ";
        std::cin >> selection;

        valid = true;

        if (selection < 0 || selection >= displays.size()) {
            std::cerr << "Invalid selection" << std::endl;
            valid = false;
        }
    }

    // Open the selected display
    auto display = Display(displays[selection]);

    std::wcout << "Selected " << display.manufacturer << " " << display.name
               << std::endl;
}
