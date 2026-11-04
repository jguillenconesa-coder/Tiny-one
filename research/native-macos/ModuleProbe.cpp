// SPDX-License-Identifier: AGPL-3.0-only
#include <CoreFoundation/CoreFoundation.h>
#include "pluginterfaces/base/ipluginbase.h"
#include <iostream>

// Load the actual VST3 bundle in a separate native process. This is not a DAW
// scanner or an audio-device test; the processor/render tests run separately.
int main(int argc, char** argv) {
    if (argc != 2) return 2;
    auto path = CFStringCreateWithCString(nullptr, argv[1], kCFStringEncodingUTF8);
    if (!path) return 2;
    auto url = CFURLCreateWithFileSystemPath(nullptr, path, kCFURLPOSIXPathStyle, true);
    CFRelease(path);
    auto bundle = url ? CFBundleCreate(nullptr, url) : nullptr;
    if (url) CFRelease(url);
    if (!bundle) { std::cerr << "Cannot create VST3 bundle\n"; return 1; }
    CFErrorRef error = nullptr;
    if (!CFBundleLoadExecutableAndReturnError(bundle, &error)) {
        if (error) { CFShow(error); CFRelease(error); }
        CFRelease(bundle);
        return 1;
    }
    auto entry = reinterpret_cast<bool (*)(CFBundleRef)>(CFBundleGetFunctionPointerForName(bundle, CFSTR("bundleEntry")));
    auto finish = reinterpret_cast<bool (*)()>(CFBundleGetFunctionPointerForName(bundle, CFSTR("bundleExit")));
    auto getFactory = reinterpret_cast<Steinberg::IPluginFactory* (*)()>(CFBundleGetFunctionPointerForName(bundle, CFSTR("GetPluginFactory")));
    bool valid = entry && finish && getFactory;
    int classes = 0;
    if (valid) {
        valid = entry(bundle);
        if (valid) {
            auto* factory = getFactory();
            classes = factory ? factory->countClasses() : 0;
            valid = classes > 0;
            if (factory) {
                for (int i = 0; i < classes; ++i) {
                    Steinberg::PClassInfo info{};
                    valid = factory->getClassInfo(i, &info) == Steinberg::kResultOk && info.name[0] && valid;
                }
                factory->release();
            }
            valid = finish() && valid;
        }
    }
    CFBundleUnloadExecutable(bundle);
    CFRelease(bundle);
    if (!valid) { std::cerr << "VST3 bundle/factory lifecycle failed\n"; return 1; }
    std::cout << "PASS: native macOS VST3 bundle loaded; lifecycle and " << classes << " factory classes\n";
}
