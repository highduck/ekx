#include <ek/app/res.hpp>
#include <ek/app/internal/res_sys.hpp>

#import <Foundation/Foundation.h>

namespace ek {

void get_resource_content_async(const char* path, const get_content_callback_func& callback) {
    NSString* asset_path = [NSString stringWithUTF8String:path];
    NSString* bundle_path = [[NSBundle mainBundle] pathForResource:asset_path ofType:nil];
    const char* file_path = [bundle_path cStringUsingEncoding:NSASCIIStringEncoding];
    callback(internal::read_file_bytes(file_path ? file_path : path));
}

}