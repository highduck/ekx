#import <UIKit/UIKit.h>

#ifndef GLES_SILENCE_DEPRECATION
#define GLES_SILENCE_DEPRECATION
#endif

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger framesPerSecond;

- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;

@end
