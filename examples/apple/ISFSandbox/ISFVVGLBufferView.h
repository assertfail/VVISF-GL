#import <AppKit/AppKit.h>
#import <pthread.h>
#import <libkern/OSAtomic.h>

#include "ISFKit.h"




using namespace VVGL;




@interface ISFVVGLBufferView : NSOpenGLView	{
	BOOL				initialized;
	pthread_mutex_t		renderLock;
	NSString			*sceneFilePath;
	VVISF::ISFSceneRef	scene;	//	this scene draws in the view
	VVGL::VVGLBufferRef	vao;
	
	VVGL::SizingMode	sizingMode;
	
	BOOL				retainDraw;
	OSSpinLock			retainDrawLock;
	VVGL::VVGLBufferRef	retainDrawBuffer;
	
	BOOL				onlyDrawNewStuff;	//	NO by default. if YES, only draws buffers with content timestamps different from the timestamp of the last buffer displayed
	OSSpinLock			onlyDrawNewStuffLock;
	VVGL::Timestamp		onlyDrawNewStuffTimestamp;
}

- (void) redraw;
///	Draws the passed buffer
- (void) drawBuffer:(VVGL::VVGLBufferRef)b;
///	Sets the GL context to share- this is generally done automatically (using the global buffer pool's shared context), but if you want to override it and use a different context...this is how.
//- (void) setSharedGLContext:(CGLContextObj)c;
- (void) setSharedGLContext:(const VVGLContextRef)n;

- (void) useFile:(NSString *)inFilePath;
- (const VVISF::ISFSceneRef &) scene;

@property (assign,readwrite) VVGL::SizingMode sizingMode;
- (void) setRetainDraw:(BOOL)n;
- (void) setRetainDrawBuffer:(VVGL::VVGLBufferRef)n;
@property (assign,readwrite) BOOL onlyDrawNewStuff;

@end







@interface NSOpenGLView (NSOpenGLViewISFVVGLBufferViewAdditions)
- (NSRect) backingBounds;
@end