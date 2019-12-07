#include "static_resources.hpp"

#import <Foundation/Foundation.h>

namespace ek {

std::vector<uint8_t> get_resource_content(const char* path) {
    NSString* asset_path = [NSString stringWithUTF8String: path];
    NSString* bundle_path = [[NSBundle mainBundle] pathForResource: asset_path ofType: nil];
    const char* file_path = [bundle_path cStringUsingEncoding: NSASCIIStringEncoding];
    return get_content(file_path ? file_path : path);
}

}