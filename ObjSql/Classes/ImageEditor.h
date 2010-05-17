//
//  ImageEditor.h
//  ObjSql
//
//  Created by John Holdsworth on 13/09/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface ImageEditor : UIImageView {
	CGContextRef bitmapContext;
	CGSize bmsize;
	char *bitmap;
	CGPoint prev;
	id delegate;
}


@end
