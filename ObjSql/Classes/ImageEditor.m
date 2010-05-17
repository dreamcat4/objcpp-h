//
//  ImageEditor.m
//  ObjSql
//
//  Created by John Holdsworth on 13/09/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "ImageEditor.h"


@implementation ImageEditor

- (void)setDelegate:(id)aDelegate {
	delegate = aDelegate;
}

- (id)delegate {
	return delegate;
}

- (void)updateView {
	CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);
	[super setImage:[UIImage imageWithCGImage:cgImage]];
	CGImageRelease( cgImage );
}

- (void)setImage:(UIImage *)image {
	if ( bitmapContext )
		CGContextRelease(bitmapContext);
	bmsize = self.frame.size;
	int bitsPerComponent = 8, bytesPerRow = bitsPerComponent/8*4*bmsize.width;
	bitmapContext = CGBitmapContextCreate ( bitmap = calloc( bytesPerRow, bmsize.height ), bmsize.width, bmsize.height,
										   bitsPerComponent, bytesPerRow, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast );		
	CGContextDrawImage( bitmapContext, CGRectMake(0.0, 0.0, bmsize.width, bmsize.height) , image.CGImage );
	[self updateView];
}

- (NSData *)text {
	CGImageRef cgImage = CGBitmapContextCreateImage(bitmapContext);
	NSData *imageData = UIImagePNGRepresentation( [UIImage imageWithCGImage:cgImage] );
	CGImageRelease( cgImage );
	return imageData;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	for ( UITouch *t in touches ) {
		prev = [t locationInView:self];
#if 0
		if ( t.tapCount > 1 && 0 ) {
			[cont undo];
			[cont undo];
		}
		else {
			[cont do];
			[self touchesMoved:touches withEvent:event];
		}
#endif
		[self touchesMoved:touches withEvent:event];
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	CGSize sz = self.frame.size;
	for ( UITouch *t in touches ) {
		CGFloat off = 20;
		CGPoint loc = [t locationInView:self];
		CGContextRef cg = bitmapContext;
		CGContextSetLineCap(cg, kCGLineCapRound);
		CGContextSetStrokeColorWithColor(cg, [UIColor blackColor].CGColor );
		CGContextSetLineWidth( cg, 2 );
		CGContextMoveToPoint( cg, (prev.x-off)*bmsize.width/sz.width, (sz.height-prev.y)*bmsize.height/sz.height );
		CGContextAddLineToPoint( cg, (loc.x-off)*bmsize.width/sz.width, (sz.height-loc.y)*bmsize.height/sz.height );
		CGContextDrawPath( cg, kCGPathStroke );
		prev = loc;
	}
	[self updateView];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesMoved:touches withEvent:event];
	[delegate performSelector:@selector(valueChanged:) withObject:self afterDelay:.1];
}

- (void)dealloc {
	CGContextRelease( bitmapContext );
	free( bitmap );
	[super dealloc];
}

@end
