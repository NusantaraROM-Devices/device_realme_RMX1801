/*
 * Copyright (C) 2020 LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <sys/sysinfo.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <android-base/properties.h>
#include <android-base/logging.h>

#include "vendor_init.h"

struct RMX180X_props
{
    std::string build_description;
    std::string build_fingerprint;
    std::string device_build;
    std::string product_device;
};

std::vector<std::string> ro_props_default_source_order = {
    "",
    "bootimage.",
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
};

void property_override(char const prop[], char const value[], bool add = true)
{
    prop_info *pi;
    pi = (prop_info *)__system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else if (add)
        __system_property_add(prop, strlen(prop), value, strlen(value));
}

void setRMX(unsigned int variant)
{
    RMX180X_props prop[3] = {};

    std::string build_desc = "raven-user 12 SQ1D.211205.016.A1 7957957 release-keys";
    std::string build_fingerprint = "google/raven/raven:12/SQ1D.211205.016.A1/7957957:user/release-keys";

    // RMX1801
    prop[0] = {
        build_desc,
        build_fingerprint,
        "RMX1801",
        "RMX1801",
    };

    // RMX1803
    prop[1] = {
        build_desc,
        build_fingerprint,
        "RMX1803",
        "RMX1803",
    };

    prop[2] = {
        build_desc,
        build_fingerprint,
        "RMX1807",
        "RMX1807",
    };

    const auto set_ro_build_prop = [](const std::string &source,
                                      const std::string &prop, const std::string &value) {
        auto prop_name = "ro." + source + "build." + prop;
        property_override(prop_name.c_str(), value.c_str(), false);
    };

    const auto set_ro_product_prop = [](const std::string &source,
                                        const std::string &prop, const std::string &value) {
        auto prop_name = "ro.product." + source + prop;
        property_override(prop_name.c_str(), value.c_str(), false);
    };

    property_override("ro.build.description", prop[variant].build_description.c_str());
    property_override("ro.build.fingerprint", prop[variant].build_fingerprint.c_str());
    property_override("ro.build.product", prop[variant].product_device.c_str());
    for (const auto &source : ro_props_default_source_order)
    {
        set_ro_build_prop(source, "fingerprint", prop[variant].build_fingerprint.c_str());
        set_ro_product_prop(source, "device", prop[variant].product_device.c_str());
        set_ro_product_prop(source, "model", prop[variant].device_build.c_str());
    }
}

void set_dalvik()
{
    // Set dalvik heap configuration
    char const *heapstartsize;
    char const *heapgrowthlimit;
    char const *heapmaxfree;
    char const *heaptargetutilization;

    struct sysinfo sys;
    sysinfo(&sys);

    if (sys.totalram > 6144ull * 1024 * 1024) {
        // from - phone-xhdpi-8192-dalvik-heap.mk
        heapstartsize = "24m";
        heapgrowthlimit = "256m";
        heaptargetutilization = "0.46";
        heapmaxfree = "48m";
        setRMX(2);
    } else if (sys.totalram > 4096ull * 1024 * 1024) {
        // from - phone-xhdpi-6144-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heaptargetutilization = "0.5";
        heapmaxfree = "32m";
        setRMX(1);
    } else {
        // from - phone-xhdpi-4096-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heaptargetutilization = "0.6";
        heapmaxfree = "16m";
        setRMX(0);
	}

    property_override("dalvik.vm.heapstartsize", heapstartsize);
    property_override("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    property_override("dalvik.vm.heapsize", "512m");
    property_override("dalvik.vm.heaptargetutilization", heaptargetutilization);
    property_override("dalvik.vm.heapminfree", "8m");
    property_override("dalvik.vm.heapmaxfree", heapmaxfree);
}

void vendor_load_properties()
{
    set_dalvik();
}
