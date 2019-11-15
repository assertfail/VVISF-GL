#ifndef VVGL_GLBuffer_hpp
#define VVGL_GLBuffer_hpp

#include "VVGL_Defines.hpp"

#include <vector>
#include <chrono>
#include <functional>
#include "GLContext.hpp"




namespace VVGL	{




//! Representation of a GL resource of some sort- most commonly an OpenGL texture, but can also be other kinds of buffers (render buffers, VBOs, EBOs, FBO, etc).
/*!
\ingroup VVGL_BASIC
Many GL objects- like buffers and textures- need to be explicitly created and deleted via gl* function calls.  All these GL calls can get tedious and overwhelming as the scale and complexity of a GL project increases- particularly if these resources need to be safely shared between contexts.  GLBuffer is an attempt to simplify that by using the lifetime of an instance of the GLBuffer class to govern the lifetime of the underlying GL resource- when the GLBuffer instance is deallocated, its underlying GL resource is returned to the GLBufferPool which created it, where it is either deleted or recycled for later use.

Notes on use:
- You should strive to work almost exclusively with #GLBufferRef, which is a std::shared_ptr around a GLBuffer.  This allows multiple objects to establish strong references to the same underlying GPU resource.
- You can't just create a GLBuffer/GLBufferRef directly via its constructor- instead, you need to use one of the creation functions listed in (\ref VVGL_BUFFERCREATE).  For more information, check out the documentation for GLBufferPool.
- Most vars are public for ease of access- instances of the GLBuffer class should be treated for the most part as read-only (member vars are populated when the underlying GL resource is created).  The only stuff you'd realistically want to change are the 'srcRect' and 'flipped' member vars.
*/
class VVGL_EXPORT GLBuffer	{
	
	public:
		//!	This defines a callback that is used to release the backing of a GLBuffer, where appropriate.  The GLBuffer being released is the first var, and the GLBuffer's backingContext is the second var.  The backingContext is probably a pointer to an object from another SDK that has been retained, and needs to be released/freed using the appropriate means.
		using BackingReleaseCallback = std::function<void(GLBuffer&, void*)>;
		
		
		//!	Describes the several different kinds of GLBuffers
		enum Type	{
			//!	CPU-only buffer
			Type_CPU,
			//!	Renderbuffer
			Type_RB,
			//!	FBO
			Type_FBO,
			//!	Texture
			Type_Tex,
			//!	Pixel Buffer Object
			Type_PBO,
			//!	%Vertex Buffer Object
			Type_VBO,
			//!	Element Buffer Object
			Type_EBO,
			//!	%Vertex Attribute Object
			Type_VAO,
		};
		
		
		//	enums describing the various GL object (usually texture) properties- split up b/c availability depends on platform
#if defined(VVGL_SDK_MAC)
		#include "GLBuffer_Enums_Mac.h"
#elif defined(VVGL_SDK_RPI)
		#include "GLBuffer_Enums_RPI.h"
#elif defined(VVGL_SDK_IOS)
		#include "GLBuffer_Enums_IOS.h"
#elif defined(VVGL_SDK_GLFW)
		#include "GLBuffer_Enums_GLFW.h"
#elif defined(VVGL_SDK_OF)
		#include "GLBuffer_Enums_OF.h"
#elif defined(VVGL_SDK_QT)
		#include "GLBuffer_Enums_Qt.h"
#elif defined(VVGL_SDK_WIN)
		#include "GLBuffer_Enums_Win.h"
#endif
		
		
		/*!
		\brief Indicates the relationship this buffer has with its backing
		*/
		enum Backing	{
			Backing_None,	//!<	There is no resource
			Backing_Internal,	//!<	The resource was created by this framework (and should be deleted by this framework)
			Backing_External	//!<	The resource was created outside of this framework, and will also be freed outside of this framework.  this buffer will probably be freed immediately (not pooled).
		};
		
		
		//! The "BackingID" is an arbitrary enum that isn't used functionally by this lib.
		/*!
		This enum- and GLBuffer's corresponding "backingID" member- exist to help track where an GLBuffer came from (if it was made from pixels, from another object, etc).  This is purely for use by other frameworks/libs for introspection about the source of the GLBuffer- some values used by this lib are listed below, you should feel free to define your own IDs where you use VVGL to create GLBuffers that wrap CPU or GPU resources vended by another API.
		*/
		enum BackingID	{
			//!	No backing ID or unknown backing ID
			BackingID_None,
			//!	The CPU backing is a GWorld, and must be freed appropriately.
			BackingID_GWorld,
			//!	The CPU backing was allocated by this library, and will be freed when the buffer's resources are no longer needed
			BackingID_Pixels,
			//!	The CPU backing was a CVPixelBufferRef allocated by CoreVideo- this GLBuffer maintains a strong reference to the CVPixelBuffer.
			BackingID_CVPixBuf,
			//!	The GPU backing was a CVOpenGLTextureRef allocated by CoreVideo- this GLBuffer maintains a strong reference to the texture ref.
			BackingID_CVTex,
			//!	The CPU backing was an NSBitmapImageRep, which will be freed when the buffer's resources are no longer needed.
			BackingID_NSBitImgRep,
			//!	The GLBuffer was created from an IOSurface (Mac SDK only) that was generated by another process
			BackingID_RemoteIOSrf,
			//!	The GLBuffer was created from an external, CPU-based source
			BackingID_GenericExternalCPU,
			//!	The CPU backing was a QImage, which will be deleted when the buffer's resources are no longer needed
			BackingID_QImage,
			//!	The CPU backing was a QVideoFrame, which will be deleted when the buffer's resources are no longer needed
			BackingID_QVideoFrame,
		};
		
		
		//!	The Descriptor struct describes the hardware attributes of the GLBuffer.
		/*!
		These are usually properties that cannot be changed, and are in their own struct for organizational purposes (buffers in the pool are checked for compatibility using equality of instances of this struct)
		*/
		struct VVGL_EXPORT Descriptor	{
			//!	What type of buffer this is (texture, renderbuffer, etc.)
			Type					type = Type_Tex;
			//!	What kind of texture this is (only used if this buffer is Type_Tex)
			Target		target = Target_2D;
			//!	The internal format of the GL texture (only used if this buffer is Type_Tex)
#if defined(VVGL_SDK_MAC)
			InternalFormat			internalFormat = IF_RGBA8;
#else
			InternalFormat			internalFormat = IF_RGBA;
#endif
			//!	The internal pixel format of the buffer (only used if this buffer is Type_Tex or Type_RB
			PixelFormat				pixelFormat = PF_RGBA;
			//!	The internal pixel type of the buffer (only used if this buffer is Type_Tex).
			PixelType				pixelType = PT_UByte;
			//!	The nature of the backing of the CPU resource for this GLBuffer.
			Backing					cpuBackingType = Backing_None;
			//!	The nature of the backing of the GPU resource for this GLBuffer.
			Backing					gpuBackingType = Backing_None;
			//!	Only used by Mac SDK, indicates platform-specific optimization.
			bool					texRangeFlag = false;
			//!	Only used by Mac SDK, indicates platform-specific optimization.
			bool					texClientStorageFlag = false;
			uint32_t				msAmount = 0;
			//!	Only used by Mac SDK, indicates platform-specific optimization.
			uint32_t				localSurfaceID = 0;
			
			//!	Returns the amount of memory in bytes that a single row of the provided width requires with the receiver's properties.
			uint32_t bytesPerRowForWidth(const uint32_t & w) const;
			//!	Returns the amount of memory in bytes required to accommodate a buffer of the passed size with the receiver's properties.
			uint32_t backingLengthForSize(const Size & s) const;
		};
	
	
	public:
		//!	Describes basic properties of the underlying resources
		Descriptor				desc;
		
		//!	The name of the OpenGL object (the texture name/renderbuffer name/VBO name/etc)
		uint32_t				name = 0;
		//!	Whether or not the GL resources associated with the GL buffer should be freed when the GL buffer is deallocated.  Defaults to 'false', which indicates that the GL resource should be pooled.
		bool					preferDeletion = false;
		//!	The size of the OpenGL buffer, in pixels (only relevant where the GLBuffer represents an image).  You should never change this value.
		Size					size = { 0, 0 };
		//!	Only relevant where the GLBuffer represents an image.  The srcRect is the region (in pixels) of the texture/renderbuffer which comprises the GLBuffer's image.  Most of the time this will be {0,0,size.width,size.height}, but it can also be a sub-region of the texture/renderbuffer, as is the case with a texture atlas.  It is safe to change this value.
		Rect					srcRect = { 0, 0, 0, 0 };
		//!	Whether or not the image in "srcRect" is flipped vertically.  Other classes in VVGL and VVISF use this in an attempt to avoid having to run additional render passes that flip upside-down assets.
		bool					flipped = false;
		//!	The size of the CPU backing, in pixels
		Size					backingSize = { 0, 0 };
		//!	The content timestamp- when the buffer was created.  Every GLBufferPool is capable of stamping the GLBuffers it vends.
		Timestamp				contentTimestamp = { static_cast<uint64_t>(0), static_cast<uint32_t>(0) };
		//!	If this is a PBO-type GLBuffer, this indicates whether or not the PBO is currently mapped, and must correspondingly be unmapped before use.
		bool					pboMapped = false;
		
		
		//!	If non-nil, this lambda/function is executed when GLBuffer's image resources are being deallocated.  This is where you want to release any CPU-based resources that are retained by the GLBuffer in its 'backingContext' member var.
		BackingReleaseCallback	backingReleaseCallback = nullptr;
		//!	This is an arbitrary pointer that is stored with the GLBuffer at its creation, and passed to the backingReleaseCallback.  If you're making a GLBuffer that uses a CPU resource provided by another SDK, this is where you store a copy of the pointer from the other SDK.
		void					*backingContext = nullptr;
		//!	Totally optional, used to describe where the backing came from.  Sometimes you want to know what kind of backing a GLBuffer has, and access it.  There's an enum describing some of the more common sources, and you can define and use your own values here.
		BackingID				backingID = BackingID_None;
		//!	If the GLBuffer has any CPU-backed resources, this is a raw pointer to the beginning of the memory that OpenGL needs to work with.  This is a weak ref, and if it's non-nil it's only valid for the lifetime of the GLBuffer.  This may point to the beginning of the block of memory that OpenGL works with, but this member var should not be used to retain any underlying resources (that role is served by the 'backingContext' member var).
		void					*cpuBackingPtr = nullptr;
	
	private:
#if defined(VVGL_SDK_MAC)
		//id						userInfo = nullptr;	//	RETAINED, nil by default.  not used by this class- stick whatever you want here and it will be retained for the lifetime of this buffer.  retained if you copy the buffer!
		//!	Mac SDK only.  The "local" surface ref was created by this process/lib.  This is a strong reference, and when this GLBuffer and its backing IOSurface are deleted the IOSurface's image will only exist in whatever remote processes have been made aware of this IOSurface.
		IOSurfaceRef			_localSurfaceRef = nullptr;
		//!	Mac SDK only.  The "remote" surface ref was created by another process (if this is non-nil, the GLBuffer should be released immediately because another process created the image resources underlying this IOSurface).
		IOSurfaceRef			_remoteSurfaceRef = nullptr;
#endif
	
	public:
		//	Every GLBuffer maintains a strong ref to the GLBufferPoolRef that created it, when the buffer is deallocated its underlying GL resources are returned to the pool where they are either released or recycled.
		GLBufferPoolRef		parentBufferPool = nullptr;
		//	If this GLBuffer was created by copying another buffer using GLBufferCopy(), the original source buffer is retained here.  If you copy a copy, the original GLBuffer is retained here.  If this is non-null, none of the resources in this buffer will be released when it is destroyed- they will only be released when the final 'copySourceBuffer' instance is destroyed!
		GLBufferRef			copySourceBuffer = nullptr;
		//	The 'copySourceBuffer' ivar above is used logically to determine if the backend resources should be released (if it is non-null then any associated backend resources will, by definition, not be freed).  Sometimes, you just want to retain one buffer with another- for example, IOSurface-backed GL textures need to be tracked in pairs, lest the "source" IOSurface get recycled and inadvertently update the other corresponding IOSurface-backed tex.  This is a simple strong relationship- 'associatedBuffer' is destroyed when its owner is destroyed.
		GLBufferRef			associatedBuffer = nullptr;
		int					idleCount = 0;
	
	
	//	public methods
	public:
		GLBuffer(GLBuffer &&) = default;
		GLBuffer() = default;
		GLBuffer(GLBufferPoolRef inParentPool);
		GLBuffer(const GLBuffer &);
		virtual ~GLBuffer();
		friend std::ostream & operator<<(std::ostream & os, const GLBuffer & n) { os << n.getDescriptionString(); return os; }
		
		//	copy assignment operators are disabled to prevent accidents
		GLBuffer& operator=(const GLBuffer&) = delete;
		//GLBuffer& operator=(GLBuffer&) = delete;
		//GLBuffer (GLBuffer&&) = default;
		
		//!	Calculates the number of bytes that are required to display one row's worth of image data from this GLBuffer.
		uint32_t calculateBackingBytesPerRow() { return desc.bytesPerRowForWidth(static_cast<uint32_t>(round(backingSize.width))); }
		//!	Calculates the total number of bytes that are required to contain this GLBuffer's image data in system memory.
		uint32_t calculateBackingLength() { return desc.backingLengthForSize(backingSize); }
		
		//	use this to create a shallow copy (memberwise copy)
		GLBuffer * allocShallowCopy();
		
#if defined(VVGL_SDK_MAC)
		//	getter/setters
		//id getUserInfo() const;
		//void setUserInfo(id n);
		IOSurfaceRef localSurfaceRef() const;
		void setLocalSurfaceRef(const IOSurfaceRef & n);
		IOSurfaceRef remoteSurfaceRef() const;
		void setRemoteSurfaceRef(const IOSurfaceRef & n);
#endif
		
		//	member methods
		bool isComparableForRecycling(const GLBuffer::Descriptor & n) const;
		uint32_t backingLengthForSize(Size s) const;
		//!	Only relevant where the GLBuffer is a texture.  This is "srcRect", but converted to the texture's coordinates (normalized if the target is GL_TEXTURE_2D, non-normalized if it's GL_TEXTURE_RECTANGLE_EXT).
		Rect glReadySrcRect() const;
		/*
		Rect croppedSrcRect(Rect & cropRect, bool & takeFlipIntoAccount) const;
		*/
		//!	Returns true if the receiver's image is full-frame (if the receiver's srcRect's origin is zero and size is the size of the reciver).
		bool isFullFrame() const;
		//!	Returns true if the receiver is a texture, if the texture target is GL_TEXTURE_2D, if the texture's dimensions are power-of-two, and if the receiver's srcRect is NOT full-frame.
		bool isNPOT2DTex() const;
		//!	Returns true if the receiver is a texture, if the texture target is GL_TEXTURE_2D, and if the texture's dimensions are power-of-two.
		bool isPOT2DTex() const;
#if defined(VVGL_SDK_MAC)
		//!	Mac SDK only.  Returns a true if the receiver is safe to publish to Syphon.
		bool safeToPublishToSyphon() const;
#endif
		
#if !defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
		/*!
		\brief Maps the PBO if the receiver is a PBO-type GLBuffer, does nothing if it's not a PBO-type GLBuffer.
		\param inAccess The access policy OpenGL will use when mapping the PBO.  GL_READ_ONLY, GL_WRITE_ONLY, or GL_READ_WRITE.
		\param inUseCurrentContext If true, the current GL context for this thread will be used to perform the GL operation.  Defaults to false (by default, it will make its buffer pool's context current before doing the GL op).
		*/
		void mapPBO(const uint32_t & inAccess, const bool & inUseCurrentContext=false);
		/*!
		\brief Unmaps the PBO if the receiver is a PBO-type GLBuffer, does nothing if it's not a PBO-type GLBuffer.
		\param inUseCurrentContext If true, the current GL context for this thread will be used to perform the GL operation.  Defaults to false (by default, it will make its buffer pool's context current before doing the GL op).
		*/
		void unmapPBO(const bool & inUseCurrentContext=false);
#endif	//	!defined(VVGL_TARGETENV_GLES) && !defined(VVGL_TARGETENV_GLES3)
		//!	Returns a true if the receiver's timestamp is a match to the passed GLBuffer's timestamp (assumes that timestamps can be used to uniquely identify the content of a GLBuffer instance).
		bool isContentMatch(GLBuffer & n) const;
		//void draw(const Rect & dst) const;
		//!	Returns a std::string that describes some basic properties of the GLBuffer instance
		std::string getDescriptionString() const;
};




//! GLBufferCopy returns a GLBufferRef for a new GLBuffer instance.  This new instance of GLBuffer actually shares the same CPU/GPU resources as the passed buffer.
/*!
\relatesalso GLBuffer
One of the uses of this function is for texture atlases- with this function you can create GLBuffers that refer to a sub-region of a larger GLBuffer/texture, which is explicitly retained.
*/
VVGL_EXPORT GLBufferRef GLBufferCopy(const GLBufferRef & n);




}




#endif /* VVGL_GLBuffer_hpp */
