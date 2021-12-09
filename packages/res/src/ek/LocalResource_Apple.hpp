#pragma once

#include "LocalResource.hpp"
#include "LocalResource_System.hpp"
#import <Foundation/Foundation.h>

namespace ek {

int getFile_platform(const char* path, LocalResource* lr) {
    NSString* asset_path = [NSString stringWithUTF8String:path];
    NSString* bundle_path = [[NSBundle mainBundle] pathForResource:asset_path ofType:nil];
    const char* file_path = [bundle_path cStringUsingEncoding:NSASCIIStringEncoding];
    return getFile(file_path ? file_path : path, lr);
}

}