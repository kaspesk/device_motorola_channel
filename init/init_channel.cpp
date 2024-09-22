#include <cstdlib>
#include <fstream>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <vector>

#include <android-base/properties.h>
#include <android-base/logging.h>
#include <init/DeviceLibinit.h>
#include "vendor_init.h"
#include "property_service.h"

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;
using android::base::SetProperty;

char const *heapstartsize;
char const *heapgrowthlimit;
char const *heapsize;
char const *heapminfree;
char const *heapmaxfree;
char const *heaptargetutilization;

std::vector<std::string> ro_props_default_source_order = {
    "",
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
};

void property_override(char const prop[], char const value[], bool add = true)
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else if (add)
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void check_device()
{
    struct sysinfo sys;
    sysinfo(&sys);

    if (sys.totalram > 5072ull * 1024 * 1024) {
        // Configurations for devices with more than 5GB of RAM
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else if (sys.totalram > 3072ull * 1024 * 1024) {
        // Configurations for devices with more than 3GB of RAM
        heapstartsize = "8m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    } else {
        // Configurations for devices with less than 3GB of RAM
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.75";
        heapminfree = "512k";
        heapmaxfree = "8m";
    }
}

void vendor_load_device_properties()
{
    std::string bootsku = GetProperty("ro.boot.hardware.sku", "");
    const auto set_ro_build_prop = [](const std::string &source, const std::string &prop, const std::string &value) {
        auto prop_name = "ro." + source + "build." + prop;
        property_override(prop_name.c_str(), value.c_str(), false);
    };

    const auto set_ro_product_prop = [](const std::string &source, const std::string &prop, const std::string &value) {
        auto prop_name = "ro.product." + source + prop;
        property_override(prop_name.c_str(), value.c_str(), false);
    };

    if (bootsku == "XT1952-T") {
        // Settings for T-Mobile REVVLRY
        property_override("ro.build.description", "channel_revvl-user 10 QPY30.85-18 6572f release-keys");
        property_override("persist.vendor.radio.customer_mbns", "tmo_usa_ims_default.mbn;sprint_usa_ims.mbn");
        property_override("persist.vendor.radio.data_con_rprt", "1");
        property_override("persist.vendor.ims.playout_delay", "10");
        property_override("persist.vendor.ims.cam_sensor_delay", "20");
        property_override("persist.vendor.ims.display_delay", "40");
        for (const auto &source : ro_props_default_source_order) {
            set_ro_build_prop(source, "fingerprint", "motorola/channel_revvl/channel:10/QPY30.85-18/6572f:user/release-keys");
            set_ro_product_prop(source, "device", "channel");
            set_ro_product_prop(source, "model", "REVVLRY");
            set_ro_product_prop(source, "name", "channel_revvl");
        }
    } else {
        // Settings for Moto G7 Play
        property_override("ro.build.description", "channel-user 9 PPY29.148-140 687ae release-keys");
        for (const auto &source : ro_props_default_source_order) {
            set_ro_build_prop(source, "fingerprint", "motorola/channel_retail/channel:9/PPY29.148-140/687ae:user/release-keys");
            set_ro_product_prop(source, "device", "channel");
            set_ro_product_prop(source, "model", "moto g(7) play");
        }
    }

    std::string device = GetProperty("ro.product.device", "");
    LOG(ERROR) << "Found bootsku '" << bootsku << "' setting build properties for '" << device << "' device\n";
}

void vendor_load_properties()
{
    check_device();

    // Setting Dalvik properties
    SetProperty("dalvik.vm.heapstartsize", heapstartsize);
    SetProperty("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    SetProperty("dalvik.vm.heapsize", heapsize);
    SetProperty("dalvik.vm.heaptargetutilization", heaptargetutilization);
    SetProperty("dalvik.vm.heapminfree", heapminfree);
    SetProperty("dalvik.vm.heapmaxfree", heapmaxfree);

    // Overriding carrier property
    std::string carrier = GetProperty("ro.boot.carrier", "unknown");
    property_override("ro.carrier", carrier.c_str());

    // Loading device-specific properties
    vendor_load_device_properties();
}
