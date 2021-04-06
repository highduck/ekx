namespace ek::apple {
void handle_exit_request();
}

#if __has_feature(objc_arc)
#define OBJC_RELEASE(obj) { obj = nil; }
#else
#define OBJC_RELEASE(obj) { [obj release]; obj = nil; }
#endif