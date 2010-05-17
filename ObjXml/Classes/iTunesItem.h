//
//  iTunesItem.h
//  ObjXml
//
//  Created by John Holdsworth on 08/10/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "objcpp.h"

@interface iTunesItem : NSObject {
	OOString title, link, description, pubDate, encoded, category, 
	artist, artistLink, album, albumLink, albumPrice, coverArt, releaseDate;
	OOData coverArtImageData;
}

@end
