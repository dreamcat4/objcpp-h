//
//  Records.h
//  DB
//
//  Created by John Holdsworth on 09/09/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "objsql.h"

@class AuthorsTitles;
@class Publishers;
@class Sales;
@class SalesDetail;

@interface Authors : OORecord {
@public
	NSString *Au_id, *au_lname;
	OOString au_fname, phone, address, city, state, country, postalcode;
	OOString _au_name; // derived field populated when record loads
	NSString *_zip; // derived field accessed by selector on demand
}

- (OOArray<AuthorsTitles *>)titles;
- (OOStringArray)blurbs;
- (UIImage *)picture;

@end

@interface Au_pix : OORecord {
@public
	OOString Au_id;
	OOData pic;
	OOString format, bytesize, pixwidth_hor, pixwidth_vert;
}
@end

@interface Blurbs : OORecord {
@public
	OOString Au_id, copy_text;
}
@end


@interface Discounts : OORecord {
@public
	OOString discounttype, Stor_id;
	int lowqty, highqty;
	double discount;
}
@end

@interface Roysched : OORecord {
@public
	OOString Title_id;
	float lorange, hirange, royalty;
}
@end

@interface Stores : OORecord {
@public
	OOString Stor_id, stor_name, stor_address, city, state, country, postalcode, payterms;
}

- (OOArray<Sales *>)orders;

@end

@interface TitlesAuthors : Authors {
@public
	OOString Title_id;
}
@end

@interface Titles : OORecord {
@public
	OOString Title_id, title, type, Pub_id;
	float price, advance;
	int total_sales;
	OOString notes, pubdate;
	BOOL contract;
}

- (OOArray<TitlesAuthors *>)authors;
- (OORef<Publishers *>)publisher;
- (OOArray<SalesDetail *>)sales;

@end

@interface SalesDetail : OORecord {
@public
	OOString Stor_id, Ord_num, Title_id;
	int qty;
	float discount;
}

- (OORef<Titles *>)title;
- (OORef<Stores *>)store;

@end

@interface Sales : OORecord {
@public
	OOString Stor_id, Ord_num, date;
}

- (OOArray<SalesDetail *>)detail;

@end

@interface Publishers : OORecord {
@public
	OOString Pub_id, pub_name, city, state;
}

- (OOArray<Titles *>)titles;

@end

@interface TitleAuthor : OORecord {
@public
	OOString Au_id, Title_id;
	int au_ord, royaltyper;
}
@end

@interface AuthorsTitles : Titles {
@public
	OOString Au_id;
}
@end

