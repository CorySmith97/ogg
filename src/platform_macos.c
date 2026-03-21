#import "platform_macos.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <string.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@interface GameView : NSView
@end

static struct {
    NSWindow          *window;
    GameView          *view;
    NSBitmapImageRep  *bitmap;
    uint32_t           width;
    uint32_t           height;
    bool               key_down[256];
    bool               key_released[256];
    bool               key_was_down[256];
    uint32_t          *pixels;
} platform_ctx;

// ─── GameView ─────────────────────────────────────────────────────────────────

@implementation GameView

- (BOOL)acceptsFirstResponder { return YES; }

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    if (!platform_ctx.bitmap) return;

    CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];
    CGContextSetInterpolationQuality(ctx, kCGInterpolationNone);

    NSImage *img = [[NSImage alloc] initWithSize:NSMakeSize(GAME_WIDTH, GAME_HEIGHT)];
    [img addRepresentation:platform_ctx.bitmap];
    [img drawInRect:self.bounds
           fromRect:NSZeroRect
          operation:NSCompositingOperationSourceOver
           fraction:1.0];
}

- (void)keyDown:(NSEvent *)event
{
    unsigned short code = event.keyCode;
    if (code < 256)
        platform_ctx.key_down[code] = true;
}

- (void)keyUp:(NSEvent *)event
{
    unsigned short code = event.keyCode;
    if (code < 256)
    {
        platform_ctx.key_down[code]     = false;
        platform_ctx.key_released[code] = true;
    }
}

@end

// ─── AppDelegate ──────────────────────────────────────────────────────────────

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [NSApp stop:nil];
    NSEvent *dummy = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSZeroPoint
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:dummy atStart:YES];
}
@end

// ─── Public API ───────────────────────────────────────────────────────────────

void platform_init(const char *name, uint32_t width, uint32_t height)
{
    platform_ctx.width  = width;
    platform_ctx.height = height;

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    AppDelegate *delegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:delegate];

    NSRect frame = NSMakeRect(0, 0, (CGFloat)width, (CGFloat)height);
    NSWindowStyleMask style = NSWindowStyleMaskTitled
                            | NSWindowStyleMaskClosable
                            | NSWindowStyleMaskMiniaturizable;

    platform_ctx.window = [[NSWindow alloc] initWithContentRect:frame
                                                      styleMask:style
                                                        backing:NSBackingStoreBuffered
                                                          defer:NO];
    [platform_ctx.window setTitle:[NSString stringWithUTF8String:name]];
    [platform_ctx.window center];

    platform_ctx.view = [[GameView alloc] initWithFrame:frame];
    [platform_ctx.window setContentView:platform_ctx.view];
    [platform_ctx.window makeFirstResponder:platform_ctx.view];
    [platform_ctx.window makeKeyAndOrderFront:nil];

    platform_ctx.pixels = calloc(GAME_WIDTH * GAME_HEIGHT, sizeof(uint32_t));

    uint8_t *planes[1] = { (uint8_t *)platform_ctx.pixels };

    platform_ctx.bitmap =
        [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:planes
                          pixelsWide:(NSInteger)GAME_WIDTH
                          pixelsHigh:(NSInteger)GAME_HEIGHT
                       bitsPerSample:8
                     samplesPerPixel:4
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                         bytesPerRow:(NSInteger)(GAME_WIDTH * 4)
                        bitsPerPixel:32];

    [NSApp activateIgnoringOtherApps:YES];
    [NSApp run];
}

void platform_handle_events(bool *quit)
{
    memset(platform_ctx.key_released, 0, sizeof(platform_ctx.key_released));

    @autoreleasepool
    {
        NSEvent *event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:[NSDate distantPast]
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES]))
        {
            [NSApp sendEvent:event];
            [NSApp updateWindows];
        }
    }

    if (![platform_ctx.window isVisible])
        *quit = true;
}

void platform_deinit(void)
{
    [platform_ctx.window close];
    platform_ctx.window = nil;
    platform_ctx.bitmap  = nil;
    free(platform_ctx.pixels);
    platform_ctx.pixels = NULL;
}

void platform_present(void)
{
    assert(platform_ctx.pixels != NULL);
    assert(renderer.pixels != NULL);
    // Use GAME dimensions, not window dimensions
    memcpy(platform_ctx.pixels, renderer.pixels,
           GAME_WIDTH * GAME_HEIGHT * sizeof(uint32_t));

    [platform_ctx.view setNeedsDisplay:YES];
    [platform_ctx.view displayIfNeeded];
}

bool is_key_down(int key)
{
    if (key < 0 || key >= 256) return false;
    return platform_ctx.key_down[key];
}

bool is_key_released(int key)
{
    if (key < 0 || key >= 256) return false;
    return platform_ctx.key_released[key];
}
