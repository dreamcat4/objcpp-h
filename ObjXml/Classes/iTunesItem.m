//
//  iTunesItem.m
//  ObjXml
//
//  Created by John Holdsworth on 08/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "iTunesItem.h"


@implementation iTunesItem

- (NSString *)description {
	return description;
}

- (NSData *)coverArtImageData {
	if ( *coverArt && !coverArtImageData )
		coverArtImageData = OOURL( coverArt ).data();
	return coverArtImageData;
}

@end
