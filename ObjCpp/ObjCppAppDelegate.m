//
//  ObjCppAppDelegate.m
//  ObjCpp
//
//  Created by John Holdsworth on 14/04/2009.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#ifdef OODEBUG
#define OOTrace OOVerify
#endif

static struct {
	char actual[1024*1024];
	const char *aptr;
	int line;
} adata;

void OOVerify( NSString *fmt, ... ) {
	va_list argp;
	va_start(argp, fmt);
	if ( adata.aptr ) {
#ifdef OODEBUG_EXPECT
		NSString *log = [[NSString alloc] initWithFormat:fmt arguments:argp];
#else
		NSString *log = nil;
#endif
		void *instance = va_arg( argp, void * );
		const char *action = va_arg( argp, const char * );
		if ( log ) {
			printf( "0x%08x %-10s - %s\n", (unsigned int)instance, action, [log UTF8String] );
			[log release];
		}
		if ( strncmp( action, adata.aptr, strlen( action ) ) != 0 ) {
			printf( "*** ACTION: %s, expected %s, line %d\n", action, adata.aptr, adata.line );
			if ( adata.actual[0] ) strcat( adata.actual, " " );
			strcat( adata.actual, action );
		}
		else if ( *(adata.aptr += strlen( action )) )
				adata.aptr++;
	}
	va_end( argp );
}

#define OOExpect( _actions ) _OOExpect( _actions, __LINE__ )

static void _OOExpect( const char *actions, int line ) {
	if ( adata.aptr && *adata.aptr )
		printf( "*** Remainder: %s, line %d\n", adata.aptr, adata.line );
	if ( adata.actual[0] ) {
		printf( "*** Unexpected: OOExpect( \"%s\" );, line %d\n", adata.actual, adata.line );
		adata.actual[0] = '\000';
	}
	adata.aptr = actions;
	adata.line = line;
}

/**
 A test class to check objects in C++ NSValue wrapper classes are recovered properly.
 */

static int constructed;

class OOClassTest {
public:
	OOClassTest() { constructed++; OOTrace( @"0x%08x %s", this, "TCONSTRUCT" ); }
	~OOClassTest() { constructed--; OOTrace( @"0x%08x %s", this, "TDESTRUCT" ); }
};

#import "ObjCppAppDelegate.h"

@implementation OOTestObjC

static int _alloced;

- init {
	if ( self = [super init] ) {
		_alloced++;
	}
	return self;
}

- (void)dealloc {
	_alloced--;
	[super dealloc];
}

@end

@class ChildRecord;

@interface ParentRecord : OORecord {
@public
	OOString ID;
	char c;
	short s;
	int i;
	float f;
	double d;
}
- (OOArray<ChildRecord *>)children;
@end

@interface ChildRecord : OORecord {
@public
	OOString ID;
	OOStringArray strs;	
}
@end

static int rcount, awoke;

@implementation ParentRecord

+ (NSString *)ooTableName { return @"PARENT_TABLE"; };
+ (NSString *)ooTableKey { return @"ID"; }

- init { rcount++; return [super init]; };
- (OOArray<ChildRecord *>)children {
	return [ChildRecord selectRecordsRelatedTo:self];
}
- (void)dealloc { rcount--; return [super dealloc]; }

@end

@implementation ChildRecord

+ (NSString *)ooTableName { return @"CHILD_TABLE"; };

- init { rcount++; return [super init]; };
- (void)awakeFromDB { awoke++; }

- (OOReference<ParentRecord *>)parent {
	return *[ParentRecord selectRecordsRelatedTo:self][0];
}

- (void)dealloc { rcount--; return [super dealloc]; }

@end

@interface iTunesItem : OORecord {
	OOString title, link, description, pubDate, encoded, category, 
	artist, artistLink, album, albumLink, albumPrice;
}
@end
@implementation iTunesItem
@end

/**
 These tests are used to validate objcpp.h for releases only. They are in need of a tidy up....
 ==============================================================================================
 */

@implementation ObjCppAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	BOOL allTests = TRUE;
	
	NSLog( @"Starting tests" );

	// basic references
	if ( allTests ) {
		OOTestObjC *obj = [[OOTestObjC alloc] init];
		assert( _alloced == 1 );

		OOExpect( "INIT RETAIN" );
		OOReference<OOTestObjC *> autoRef = obj;
		assert( _objcpp.retained == 1 );
		[obj release];
		assert( _alloced == 1 );
		
		OOExpect( "RETAIN" );
		ivarRef = autoRef;
		assert( _objcpp.retained == 2 );

		OOExpect( "DESTUCT RELEASE" );
	}
	assert( _objcpp.retained == 1 );
	assert( _alloced == 1 );
	
	OOExpect( "INIT RETAIN DESTUCT RELEASE DESTUCT RELEASE" );
	~ivarRef;
	OOExpect( NULL );

	assert( _objcpp.retained == 0 );
	assert( _alloced == 0 );

	// array references 
	if ( allTests ) {
		{
			OOExpect( "INIT" );
			OOArray<OOTestObjC *> autoArray;

			for ( int i=0 ; i<10 ; i++ ) {
				OOTestObjC *obj = [[OOTestObjC alloc] init];
				
				if ( i==0 )	OOExpect("ALLOC RETAIN");
				autoArray[i] = obj;
				[obj release];
			}
			assert( _objcpp.retained == 1 );
			assert( _alloced == 10 );

			OOExpect( "RETAIN" );
			ivarArray = autoArray;
			assert( _objcpp.retained == 2 );

			OOExpect( "DESTUCT RELEASE" );
		}
		assert( _objcpp.retained == 1 );
		{
			OOExpect( "INIT RETAIN" );
			OOArray<OOTestObjC *> autoArray = ivarArray;
			assert( _objcpp.retained == 2 );
			assert( _alloced == 10 );

			OOExpect( "DESTUCT RELEASE" );
		}
	}
	assert( _objcpp.retained == 1 );
	assert( _alloced == 10 );

	OOExpect( "RELEASE" );
	ivarArray = nil;
	assert( _objcpp.retained == 0 );
	assert( _alloced == 0 );


	// dictionary references 
	if ( allTests ) {
		{
			OOExpect( "INIT" );
			OODictionary<OOTestObjC *> autoDict;
			for ( int i=0 ; i<10 ; i++ ) {
				OOTestObjC *obj = [[OOTestObjC alloc] init];
				if ( i==0 )	OOExpect("ALLOC RETAIN");
				autoDict[[NSString stringWithFormat:@"K%d", i]] = obj;
				[obj release];
			}
			assert( _objcpp.retained == 1 );
			assert( _alloced == 10 );

			OOExpect("INIT RETAIN");
			OODictionary<OOTestObjC *> autoDict2 = autoDict;
			assert( _objcpp.retained == 2 );
			assert( _alloced == 10 );

			OOExpect("RETAIN");
			ivarDict = autoDict2;

			OOExpect("DESTUCT RELEASE DESTUCT RELEASE");
		}
	}
	assert( _objcpp.retained == 1 );
	assert( _alloced == 10 );

	OOExpect( "INIT RETAIN DESTUCT RELEASE DESTUCT RELEASE" );
	~ivarDict;
	assert( _objcpp.retained == 0 );
	assert( _alloced == 0 );

	// mixed subscripts
	if ( allTests ) {
		OOExpect( "INIT" );
		OODictionary<OOTestObjC *> autoMixed;
		for ( int i=0 ; i<10 ; i++ ) {
			OOTestObjC *obj = [[OOTestObjC alloc] init];
			OOExpect( i==0 ? "VIVIFY VIVIFY ALLOC RETAIN" : "VIVIFY VIVIFY" );
			autoMixed[[NSString stringWithFormat:@"K%d", i]][i][i] = obj;
			[obj release];
		}
		OOExpect("DESTUCT RELEASE");
		///OOPrint( autoMixed );
	}
	assert( _objcpp.retained == 0 );
	assert( _alloced == 0 );
	
	// string class
	if ( allTests ) {
		OOExpect("INIT");
		OOString str;

		OOExpect("ALLOC RETAIN");
		str += @"Hello World";

		OOExpect("INIT RETAIN INIT COPY RETAIN INIT COPY RETAIN INIT COPY RETAIN INIT COPY RETAIN RETAIN "
				 "RELEASE DESTUCT RELEASE DESTUCT RELEASE DESTUCT RELEASE DESTUCT RELEASE DESTUCT RELEASE" );
		str = str+@" and "+99.5+" "+123;

		OOExpect("INIT RETAIN DESTUCT RELEASE");
		str += "!";

		OOExpect("INIT RETAIN INIT RETAIN");
		OOString str2 = @"Not correct", str3 = @"Hello World and 99.500000 123!";

		assert( str != str2 );
		assert( str == str3 );
		assert( str2 > str3 );
		assert( str >= str3 );
		assert( _objcpp.retained == 3 );

		OOExpect("RETAIN");
		ivarString = str;

		OOExpect("DESTUCT RELEASE DESTUCT RELEASE DESTUCT RELEASE");
	}
	assert( _objcpp.retained == 1 );
	OOExpect("RELEASE");
	ivarString = nil;
	assert( _objcpp.retained == 0 );

	// vectors
	if ( allTests ) {
		{
			OOExpect( "INIT" );
			OOVector<double> autoVector;

			OOExpect( "CALLOC BCONSTRUCT RETAIN BREALLOC BREALLOC BREALLOC BREALLOC BREALLOC" );
			for ( int i=0 ; i<100 ; i++ )
				autoVector[i] = i*99.;
			assert( _objcpp.retained == 1 );

			for ( int i=0 ; i<100 ; i++ )
				assert( autoVector[i] == i*99. );

			OOExpect( "RETAIN" );
			ivarVector = autoVector;
			assert( _objcpp.retained == 2 );

			OOExpect( "DESTUCT RELEASE DESTUCT" );
		}
		assert( _objvec.buffered == 1 );
		for ( int i=0 ; i<100 ; i++ )
			assert( ivarVector[i] == i*99. );
		
		// 2-d matrix
		OOExpect( "INIT" );
		OOMatrix<int> mx;

		OOExpect( "CALLOC BCONSTRUCT RETAIN "
				 "BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT "
				 "BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT BCONSTRUCT "
				 "BREALLOC BREALLOC" );
		mx[1][1000] = 99;
		assert( mx[1][1000] == 99. );

		OOExpect( "CDESTOY BDESTRUCT BDESTRUCT DESTUCT RELEASE DESTUCT" );
	}
	assert( _objcpp.retained == 1 );

	OOExpect( "INIT RETAIN DESTUCT RELEASE CDESTOY BDESTRUCT DESTUCT RELEASE DESTUCT" );
	~ivarVector;

	assert( _objvec.buffered == 0 );
	assert( _objcpp.retained == 0 );

	// vector hash
	if ( allTests ) {
		OOExpect( "INIT" );
		OOClassDict<OOVector<double> > classDict;

		OOExpect( "INIT BREALLOC BCONSTRUCT RETAIN INIT RETAIN ALLOC RETAIN DESTUCT RELEASE DESTUCT" );
		classDict[@"ONE"] = new OOVector<double>(100);

		OOExpect( "INIT INIT RETAIN DESTUCT RELEASE DESTUCT" );
		classDict[@"TWO"] = new OOVector<double>;

		OOExpect( "INIT RETAIN INIT RETAIN DESTUCT RELEASE DESTUCT" );
		OOVector<double> vector = **classDict[@"ONE"];
		vector[99] = 43;
		vector[99]++;

		OOExpect( "INIT RETAIN DESTUCT RELEASE DESTUCT " );
		assert( (**classDict["ONE"])[99] == 44 );
		assert( _objvec.buffered == 1 );

		OOExpect( "DESTUCT RELEASE DESTUCT INIT COPY RETAIN INIT RETAIN CDESTOY CDESTOY BDESTRUCT "
				 "DESTUCT RELEASE DESTUCT DESTUCT RELEASE DESTUCT INIT RETAIN CDESTOY "
				 "DESTUCT DESTUCT DESTUCT RELEASE DESTUCT DESTUCT RELEASE RELEASE DESTUCT" );
	}
	assert( _objvec.buffered == 0 );
	assert( _objcpp.retained == 0 );

	// vector hash
	if ( allTests ) {
		OOExpect( "INIT" );
		OOClassArray<OOVector<double> > classArray;

		OOExpect( "INIT BREALLOC BCONSTRUCT RETAIN INIT RETAIN ALLOC RETAIN DESTUCT RELEASE DESTUCT" );
		classArray[1] = new OOVector<double>(100);

		OOExpect( "INIT INIT RETAIN DESTUCT RELEASE DESTUCT" );
		classArray[2] = new OOVector<double>();

		OOExpect( "INIT RETAIN CALLOC BCONSTRUCT RETAIN BREALLOC DESTUCT RELEASE DESTUCT" );
		(**classArray[2])[0] = 0;

		OOExpect( "INIT RETAIN INIT RETAIN DESTUCT RELEASE DESTUCT" );
		OOVector<double> vector = **classArray[2];

		OOExpect( "BREALLOC" );
		vector[99] = 43;

		OOExpect( "INIT RETAIN DESTUCT RELEASE DESTUCT" );
		assert( (**classArray[2])[99] == 43 );
		assert( _objvec.buffered == 2 );

		OOExpect( "DESTUCT RELEASE DESTUCT INIT RETAIN CDESTOY CDESTOY BDESTRUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT RELEASE DESTUCT INIT RETAIN CDESTOY CDESTOY BDESTRUCT DESTUCT "
				 "RELEASE DESTUCT DESTUCT RELEASE DESTUCT INIT DESTUCT DESTUCT RELEASE DESTUCT" );
	}
	assert( _objvec.buffered == 0 );
	assert( _objcpp.retained == 0 );
	

	// object lists
	if ( allTests ) {
		OOExpect( "INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT "
				 "INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT "
				 "INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT "
				 "INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT "
				 "INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT "
				 "INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT "
				 "INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT INIT BREALLOC BCONSTRUCT RETAIN" );
		OOObjects<OOTestObjC *> autoObjects(100);

		for ( int i=0 ; i<10 ; i++ ) {
			OOTestObjC *obj = [[OOTestObjC alloc] init];
			OOExpect( "RETAIN" );
			autoObjects[i*10] = obj;
			[obj release];
		}
		assert( _alloced == 10 );

		OOExpect( "CDESTOY BDESTRUCT "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT DESTUCT RELEASE "
				 "DESTUCT RELEASE DESTUCT" );
	}
	assert( _objcpp.retained == 0 );
	assert( _alloced == 0 );
	
	// C++ class instance hash
	if ( allTests ) {
		OOExpect( "INIT" );
		OOClassDict<OOClassTest> classDict;
		for ( int i=0 ; i<10 ; i++ ) {
			OOExpect( i==0 ? 
					 "TCONSTRUCT INIT RETAIN ALLOC RETAIN DESTUCT RELEASE DESTUCT" :
					 "TCONSTRUCT INIT RETAIN DESTUCT RELEASE DESTUCT" );
			classDict[[NSString stringWithFormat:@"K%d", i]] = new OOClassTest();
		}
		assert( constructed == 10 );

		OOExpect( "RETAIN" );
		ivarClassDict = classDict;
		OOExpect( "RELEASE DESTUCT" );
	}
	assert( constructed == 10 );

	OOExpect( "INIT RETAIN RELEASE INIT COPY RETAIN "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "INIT RETAIN CDESTOY TDESTRUCT DESTUCT RELEASE DESTUCT "
			 "DESTUCT RELEASE RELEASE DESTUCT" );
	~ivarClassDict;
	assert( constructed == 0 );
	
	OOExpect(NULL);
	
	if ( allTests ) {
		OOStrings s( 100 );
		for ( int i=0 ; i<10 ; i++ )
			s += "str";
		assert( (int)s == 10 );		
	}

	//for testing Catcher.m
	if ( 0 ) {
		OOArray<NSString *> t;
		[t.alloc() objectAtIndex:1000];
	}
	
	// string dictionary 
	if ( allTests ) {
		OOStringDict stringDict;
		for ( int i=0 ; i<10 ; i++ )
			stringDict[[NSString stringWithFormat:@"K%d", i]][i][i] = "Hello";
		for ( int i=0 ; i<10 ; i++ )
			stringDict[[NSString stringWithFormat:@"K%d", i]][i][i] += " World";
		for ( int i=0 ; i<10 ; i++ )
			*stringDict[[NSString stringWithFormat:@"K%d", i]][i][i] += @"!";
		for ( int i=0 ; i<10 ; i++ )
			*stringDict[[NSString stringWithFormat:@"K%d", i]][i][i] += i;

		assert( stringDict["K0"][0][0] == @"Hello World!0" );
		OOString str = @"Hello World!";
		for ( int i=0 ; i<10 ; i++ )
			assert( (*stringDict[[NSString stringWithFormat:@"K%d", i]][i][i] == str+i) );

		*stringDict["empty2"] += @"vivifys?";
		assert( !*stringDict["empty2"] );
		stringDict["empty1"] += @"vivifys?";
		OOString s2 = *stringDict["empty1"];
		////assert( stringDict["empty1"] == @"vivifys?" );
		stringDict["empty0"]["123"][99]["d"] += @"vivifys?";
		////assert( stringDict["empty0"]["123"][99]["d"] == "vivifys?" );
	}
	assert( _objcpp.retained == 0 );
	
	// string operators
	if ( allTests ) {
		OOString str;
		str <<= @"Hello World";
		assert( str[1] == 'e' );
		str[1] = 'a';
		assert( str[1] != 'e' );
		str[@"o"] = @"aa";
		OOString str2 = @"Hallaa Waarld";
		assert( str == str2 );
		OOString l = @"l";
		NSString *s = l;
		str <<= str - l;
		str <<= str - s;
		str <<= str - "l";
		str += ";;";
		str2 = @"Haaa Waard;;";
		assert( str == str2 );
		str <<= @"";
		OOStringArray a(0);
		assert( !a );
		OOStringArray b(nil);
		assert( !b );
		OOStringArray c(OONil);
		assert( !c );
		assert( (int)a==0 );
	
		OOArray<NSString *> stringArray;
		stringArray[0] = @"ONE";
		stringArray[1] = @"TWO";
		
		OODictionary<NSString *> stringDict;
		stringDict[@"KEY1"] = @"STRING1";
		stringDict[@"KEY2"][@"SUBKEY1"] = @"STRING2";
		str <<= @"Hello World";
		if ( str[1] == 'e' )
			str[1] = 'a';
		str = str + @"!";
		str += @"!";
		if ( str != (OOString)@"Hallo World!!" )
			assert(0);
	}
	
	// array operators
	if ( allTests ) {
		OOString str = @"A C B";
		NSString *s1 = str;
		NSMutableString *s2 = str;
		OOStringArray a1 = (const char *)str, a2 = "C B A";
		assert( a1 != a2 );
		// reverse sort
		a1 = -+a1;
		assert( a1 == a2 );
		~a1[1];
		assert( a1 != a2 );
		~a2[-2];
		assert( a1 == a2 );
		OOArray<OOString> a3 = a2;
		assert( a1 == a3 );
		NSArray *a = a2;
		NSMutableArray *m = a2;
		assert( a1 == a );
		assert( a1 == m );
		assert( (int)a1 == 2 );
		a1 += a;
		assert( (int)a1 == 4 );
		a1 += m;
		assert( (int)a1 == 6 );
		a1 += str;
		assert( (int)a1 == 7 );
		a1 += s1;
		assert( (int)a1 == 8 );
		a1 += s2;
		assert( (int)a1 == 9 );
		a1 += "1 2 3";
		assert( (int)a1 == 10 );
		a1 -= a2;
		~a1[str];
		assert( (int)a1 == 3 );
		a1 -= s1;
		assert( (int)a1 == 1 );
		a1 = a;
		a1 = m;
	}
	
	// set operators
	if ( allTests ) {
		OOStringArray a1 = "A B C", a2 = "C D E", a3 = "C", a4 = "A B C D E", a5 = kCFNull;
		assert( (a1 & a2) == a3 );
		assert( (a1 | a2) == a4 );
		a1 -= 1;
		a1 -= OOString(@"A");
		assert( a1 == OOStrArray( "C" ) );
		OOStringDict aa = OONil;
		assert( !aa );
		OOStringArray bb = nil;
		assert( !bb );
		OOObjects<NSObject *> m, n=m;
		OOStrings p, q=p;
		
		OOStringDict a = "a 1 b 2", b = "b 3 c 4";
		assert( +OOStringArray( a & b ) == "2 b" );
		assert( +OOStringArray( a | b ) == "1 2 4 a b c" );
		assert( +OOStringArray( b | a ) == "1 3 4 a b c" );
	}
	
	// array merge
	if ( allTests ) {
		OOStringArray a1 = "A B C", a2 = "D E", a3 = a1;
		a3 = a2;
		a1 += a2;
		assert( a1 == OOStrArray( "A B C D E" ) );	
		assert( a1+a1 == OOStrArray( "A B C D E" )*2 );
		if ( 0 ) {
			NSString *s = nil;
			NSMutableString *m = nil;
			NSArray *aa = nil;
			NSMutableArray *ma = nil;
			OOString os, o1 = s, o2 = m, o3 = os;
			OOStringArray oa, sa1 = aa, sa2 = ma, sa3 = oa;
			os = nil; os = s; os = m;
			aa = nil; oa = aa; oa = ma; 
			OOArray<OOString> as, s1 = aa, s2 = ma, s3 = aa, s4 = as;
			OODictionary<OOString> ds;
			as = nil;
			ds = nil;
		}
	}
	
	// type conversion
	if ( allTests ) {
		NSMutableArray *in = [NSMutableArray arrayWithObjects:@"1", @"2", @"3", nil];
		OOStringArray arr = in;
		NSMutableArray *ref = arr;
		assert( ref == in );
		ref = *arr;
		assert( ref == in );
		assert( [ref retainCount] == 2 );
		ref = &~arr; // [[*arr retain] autorelease]; //
		assert( ref == in );
		assert( [ref retainCount] == 2 );
	}
	
	if ( allTests ) {
		OOString str;
		str <<= @"Hello World";
		if ( str[1] == 'e' )
			str[1] = 'a';
		str = str + @"!";
		str += @"!";
		assert( str == (OOString)@"Hallo World!!" );

		OODictionary<OOString> stringDict;
		for ( int i=0 ; i<10 ; i++ ) {
			stringDict[@"KEY"][i] <<= @"";
			stringDict[@"KEY"][i] += "Hello ";
			stringDict[@"KEY"][i] += "World!";
		}
		
		assert( str == str );
		assert( !(str != str) );
		if ( str == (OOString)@"Hallo World!!" )
			assert( 1 );
		if ( str != (OOString)@"Hallo World!!" )
			assert( 0 );
		
		NSString *s = @"Hi";
		NSMutableString *m = (NSMutableString *)@"Ho";
		if ( 0 ) {
			assert( str == s );
			assert( str != s );
			assert( str < s );
			assert( str > s );
			assert( str <= s );
			assert( str >= s );

			assert( str == m );
			assert( str != m );
			assert( str < m );
			assert( str > m );
			assert( str <= m );
			assert( str >= m );
		}

		str = s;
		str = m;
		assert( _objcpp.retained == 2 );
		
		str = str + str;
		str = str + s;
		str = str + m;
		str = str + 1;
		str = str + 1.;

		str += str;
		str += s;
		str += m;
		str += 1;
		str += 1.;
	}
	
	// slice assign
	if ( allTests ) {
		OOStringDict d = "a 1 b 2 c 3";
		d[OOSlice("c d e")] = OOSlice( "4 5 6" );
		assert( d[OOSlice("b c d")] == OOStringArray( "2 4 5" ) );
		assert( ~d[OOSlice("a b")] == OOStringArray( "1 2" ) );
		OOStringArray a = "1 2 3 4";
		d[OOSlice("c d e")] = d[OOSlice("a b c")];

		a[NSMakeRange(0,2)] = OOStringArray( "5 6" );
		assert( a[NSMakeRange(1,2)] == OOStringArray( "6 3" ) );
		assert( ~a[NSMakeRange(2,2)] == OOStringArray( "3 4" ) );
		assert( a == OOStringArray( "5 6" ) );
		a[NSMakeRange(0,2)] = a[NSMakeRange(1,2)];
		
		OOStringDict e;
		e[@"0"] = OOStringArray("1 2 3");
		OOStringArray f = e["0"];
		assert( f == "1 2 3" );
		assert( (int)f == 3 );
		assert( OOStringArray(e["0"]) == "1 2 3" );
	}
	
	if ( allTests ) {
		// string ops
		OOString str;
		str = "Hello";
		str += @" ";
		str = str + @"World";
		str += "!";
		
		str[NSMakeRange(0,5)] = @"Mellow";
		str[@"ll"] = @"";
		
		// dictionary access
		OOStringDict dict;
		dict[@"key1"] = str;
		dict[@"key1"] += @"!";
		*dict[@"key1"] += @"!";
		assert( dict[@"key1"] == OOString(@"Meow World!!!") );
		NSString *s = &dict["key1"];
		assert( dict["key1"] == s );
	
		// recurive data structures
		dict["key2"][0] = "A0";
		dict["key2"][1] = OOString(@"A1"); ////
		assert( OOStringArray(dict["key2"]) == OOStrArray( "A0 A1" ) );
		//NSLog( @"%@", *dict );

		OOString n(99.5);
		int i = n;
		assert( i==99 );
		
		if ( 1 ) {
			OOStringArray a2( dict["key2"] );
			NSArray *a = a2;
			NSMutableArray *n = a2;
			dict -= a2;
			dict -= a;
			dict -= n;
		}
		if ( 1 ) {
			dict["a"]["b"] <<= @"1";
			dict["a"]["b"] = "1";
			dict["a"]["c"] = OOString(@"1");////
			OOStringDict d2 = dict["a"], d3 = "a 1 b 2";
			NSDictionary *d = d3;
			NSMutableDictionary *m = d3;
			assert( dict == dict );
			if (0) {
				assert( dict == d );
				assert( dict == m );
			}
			///dict += d;
			///dict += m;
			dict = m;
			dict = d;
		}
	}
	
	// assignment
	if ( allTests ) {
		NSMutableDictionary *d = nil;
		NSMutableArray *r = nil;
		OOArray<id> a;
		OODictionary<id> b = a, f = d;
		OOArray<id> c = b, e = a, g = r;
		a = *b;
		b = *a;

		// messaging boxed objects
		OOString s = 99.5;
		assert( [a count] == 0 );
		assert( [*s doubleValue] == 99.5 );
		assert( [s doubleValue] == 99.5 );

		OOArray<NSString *> x = "99.5";
		assert( [x[0] doubleValue] == 99.5 );
		assert( [*x[0] doubleValue] == 99.5 );

		OOArray<OOString> y = "99.5";
		// compiler not this clever...
		//assert( [y[0] doubleValue] == 99.5 );
		assert( [*y[0] doubleValue] == 99.5 );
		assert( [**y[0] doubleValue] == 99.5 );

		// does give warning on compile
		//if ( 0 ) [*x[0] count];
		//if ( 0 ) [**y[0] count];

		// compiles but should give warning
		if ( 0 ) {
			[a doubleValue];
			[s count];
			[x[0] count];
			[*y[0] count];
		}
	}
	
	if ( allTests ) {
		OOStringArray a = "1 2 3";
		NSMutableString *m = [[@"1" mutableCopy] autorelease];
		NSString *s = m;
		OOString o = m;
		assert( a[0] == "1" );
		//assert( *a[0] == @"1" );
		assert( a[0] == s );
		assert( a[0] == m );
		assert( a[0] == o ); 
		o = s = m = (NSMutableString *)@"2";
		assert( a[0] != "2" );
		assert( a[0] != s );
		assert( a[0] != m );
		assert( a[0] != o );
		assert( a[0] < "2" );
		assert( a[0] < s );
		assert( a[0] < m );
		assert( a[0] < o );
		o = s;
		o = m;
		o = a[0];
		a[3][0] = "ok";
		o = a[-1][-1];
		assert( o=="ok" );
	}
	
	if( allTests ) {
		OOString a = "a", b = @"b", c = a;
		a = "a";
		a = @"a";
		a = c;
	}
	
	// append
	if( allTests ) {
		OOStringArray a = "1 2 3";
		a += a;
		a += *a;
		a += "5";
		assert( a == OOStrArray( "1 2 3 1 2 3 1 2 3 1 2 3 5" ) );
	}
	
	// slices
	if ( allTests ) {
		OOStringArray z;
		z[0][0] = OOStrArray( "1 2 3" );
		assert( [**z[0][0][0] isEqualTo:@"1"] );
		assert( z[0][0][0] == @"1" );
		assert( z[0][0][NSMakeRange(1,2)] == OOStrArray( "2 3" ) );

		OOStringDict d( "a 1 b 2 c 3" );
		OOSlice a( "a b c" );
		OOStringArray n( "1 2 3" ), r = d[a];
		r = d[a];
		assert( r == n );
		assert( d[a] == n );
	}
	
	if ( allTests ) {
		NSMutableString *str;
		str = [[@"Hello" mutableCopy] autorelease];
		[str appendString:@" "];
		str = [[[str stringByAppendingString:@"World"] mutableCopy] autorelease];
		[str appendString:@"!"];
	
		// dictionary access
		NSMutableDictionary *dict = [NSMutableDictionary dictionary];
		[dict setObject:str forKey:@"key1"];
		[dict setObject:[[[[dict objectForKey:@"key1"] stringByAppendingString:@"!"] mutableCopy] autorelease] forKey:@"key1"];
		[[dict objectForKey:@"key1"] appendString:@"!"];
		
		// recursive data structures
		if ( [dict objectForKey:@"key2"] == nil )
			[dict setObject:[NSMutableArray array] forKey:@"key2"];
		[[dict objectForKey:@"key2"] insertObject:@"A0" atIndex:0];
		[[dict objectForKey:@"key2"] insertObject:@"A1" atIndex:1];
		//NSLog( @"%@", dict );
	}
	
	if ( allTests ) {
		OOString x, y = OONil, yy = @"123";
		assert( !y );
		OOStrs xs = "1 2 3";
		x += 1;
		x += 1.;
		x += "x";
		x += @"y";
		x -= "y";
		x -= @"x";
		x += self;
		x += xs[0];
		x -= xs[0];
		x = xs[0];
		assert( OOStr("abc")-"b" == "ac" );
		assert( OOStr("abc")+5 == "abc5" );
		assert( OOStr("ab44c")-"4" == "abc" );
		OOStringArray z( "a b c" );
		OOString s = z;
		s -= " ";
		assert( s=="abc" );
		s[1] = 'd';
		assert( s[0] == 'a' );
		assert( s=="adc" );
		s[OORange(2,3)] = "ddd";
		assert( s=="adddd" );
		s[OORange(1,5)] = @"";
		assert( s=="a" );
		NSString *a = @"";
		NSMutableString *b = [[@"" mutableCopy] autorelease];
		s <<= @"";
		s += "";
		s = a;
		s += "";
		s = b;
		s += "";
		s = s + s;
		s += @"";
		s += "";
		OOStringDict d = "a 1";
		s = z[0];
		s = d["a"];
	}
	
	if ( allTests ) {
		OOStringDict d = "a 1 b 2 c 3", e( @"_", *d, nil );
		OOStringArray a( *d, nil ), b = d;
		OOString s = OOStringArray( d );
		assert( d[OOSlice("a b")]+OOStrs("4") == "1 2 4" );
		assert( d[OOSlice("a b")][1] == "2" );
		assert( d[OOSlice("a b")][OORange(1,2)] == "2" );
		//a[0] = d;
		assert( a[0][OOSlice("a b")]+OOStrs("4") == OOStrs( "1 2 4" ) );
		assert( a[0][OOSlice("a b")][1] == "2" );
		assert( a[0][OOSlice("a b")][OORange(1,2)] == "2" );
		a[1] = a[0][OOSlice("a b")];
		a[2] = a[0][OOSlice("a b")][OORange(1,2)];
		assert( e["_"][OOSlice("a b")]+OOStrs("4") == OOStrs( "1 2 4" ) );
		assert( e["_"][OOSlice("a b")][1] == "2" );
		assert( e["_"][OOSlice("a b")][OORange(1,2)] == "2" );		
		e["_"] = d;
		assert( e["_"][OOSlice("a b")]+OOStrs("4") == OOStrs( "1 2 4" ) );
		assert( e["_"][OOSlice("a b")][1] == "2" );
		assert( e["_"][OOSlice("a b")][OORange(1,2)] == "2" );		
	}
	
	// regular expressions
	if ( allTests ) {
		assert( (OOStr("a123 b123 B345")|@"/([ab])(\\d+)/$1=$2/i") == OOStr("a=123 b=123 B=345") );
		assert( OOStringArray( @"a", @"b", @"c", nil ) == (OOStr( "----a----b--c----") & "\\w+") );
		assert( OOStringArray( @"a", @"b", @"c", nil ) == (OOStr( "----a----b--c----") & "\\w+") );
		assert( OOStringArray( @"a", @"b", @"c", nil ) == (OOStr( "----a----b--c----") & "\\w+") );
		assert( OOStringArray( @"a", @"b", @"c", nil ) == (OOStr( "----a----b--c----") & "\\w+") );
		assert( OOStringArray( @"", @"a", @"b", @"c", @"", nil ) == (OOPattern( "-+" ).split( "-----a----b--c----")));
		assert( OOStringArray( @"a", @"b", @"c", nil ) == (OOStr( "----a----b--c----") ^ "\\W+(\\w+)\\W+(\\w+)\\W+(\\w+)\\W+")[NSMakeRange(1,3)] );
		OOStringArray p = OOStr( "123abc" )^"(\\d+)(\\w+)(===)?";
		assert( !!p[2] );
		assert( *p[2] );
		assert( !p[3] );
		assert( !*p[3] );
		assert( !**p[3] );
	}
	
	if ( allTests ) {
		NSArray *x = nil;
		NSMutableArray *m = nil;
		OOReference<NSArray *> y = x, z = y;
		y = x;
		z = z;
		y <<= x;
		y <<= z;
		OOStringArray a = x, b = m;
		a = m;
		b = x;
		OOString s = @"1";
		s *= 3;
		assert( s == "111" );
		assert( s*2 == "111111" );
		//assert( s*0 == "" );
		OOStringArray q = "1 2";
		q *= 2;
		assert( q*2=="1 2 1 2 1 2 1 2" );
	}
	
	if ( allTests ) {
		OONumber a = 22, b = a;
		assert( a+b == 44 );
		a += b;
		a = a/2.;
		OONumberDict d;
		d["a"] = a+b;
		d["a"] += 2;
		assert( d["a"] == 46. );
	}
	
	if (allTests ) {
		OOStr a = "a", b = "b", c = "a";
		OOStrDict d;
		d["1"] = a;
		d["2"] = b;
		d["3"][0] = c;
		assert( a<b );
		assert( a==c );
		assert( d["2"]>d["1"] );
		assert( !(d["3"][0]!=d["1"]) );
		assert( (d["3"][0]<=d["1"]) );
		OOObjects<NSString *> var;
		var[99] = @"THIS";
		OOStrings ted;
		ted[99] <<= @"Go on ";
		ted[99] *= 10; 
	}
	
#if 0
	if ( allTests ) {
		OOScan s( "123abc" );
		assert( !!(s & @"123") );
		assert( !(s & @"123") );
	}
#endif
	
	// strict and ||
	if ( allTests ) {
		OOReference<NSString *> a = (id)kCFNull;
		assert( !a );
		assert( a ? 1 : 0 );
		assert( !a );
		if ( a ) ; else assert( 0 );
		assert( a ? 1 : 0 );
		a = @"1";
		assert( a );
		OOReference<NSString *> b;
		assert( !b );
		////assert( b ? 1 : 0 );
		////assert( !!b );
		////assert( b ? 1 : 0 );
		b = @"1";
		assert( b );
		OOString c = "a", d;
		assert( (c || d) == "a" );
		assert( (d || c) == "a" );
		assert( (c || "b") == "a" );
		assert( (d || "b") == "b" );
	}
	
	// slice tests
	if ( allTests ) {
		OOStringArray strings = "a b c d";
		assert( strings[NSMakeRange(1,2)] == "b c" );
		strings[OORange(1,3)] = OOStringArray( "x y" );
		assert( strings == "a x y d" );
		
		OOStringDict dict = "a 1 b 2 c 3 d 4";
		assert( dict[OOSlice("b c")] == "2 3" );
		dict[OOSlice("b c")] = OOStringArray( "9 9" );
		assert( dict[OOSlice("b c")] == "9 9" );
	}
	
	// string search
	if ( allTests ) {
		OOString a = "1abc", a0 = "0abc", b;
		assert( a );
		assert( a ? 1 : 0 );
		assert( !a ? 0 : 1 );
		assert( a0 );
		assert( a0 ? 0 : 1 );
		assert( !a0 ? 0 : 1 );
		assert( !b );
		assert( b ? 0 : 1 );
		// problems...
		assert( a[@"a"] );
		assert( a[@"a"] ? 1 : 0 );
		assert( a[@"1"] ? 0 : 1 );
		assert( !a[@"1"] ? 0 : 1 );
		assert( !a[@"z"] );
		assert( a[@"z"] ? 1 : 0 );
		assert( !b[@"z"] );
		assert( b[@"z"] ? 0 : 1 );
	}
	
	// objsql tests
	if ( allTests ) {
		[OODatabase exec:"drop table if exists PARENT_TABLE"];
		[OODatabase exec:"drop table if exists CHILD_TABLE"];

		// populate parent a child records
		for ( int i=0 ; i<10 ; i++ ) {
			ParentRecord *p = [[ParentRecord alloc] init];
			p->ID = OOString("ID")+i;
			p->c = 123*i;
			p->s = 234*i;
			p->i = 345*i;
			p->f = 456.*i;
			p->d = 567.*i;
			[[p insert] release];
			for ( int j=0 ; j<i ; j++ ) {
				ChildRecord *c = [ChildRecord insertWithParent:p];
				for ( int k=0 ; k<2 ; k++ )
					c->strs += c->ID;
			}
			assert( [OODatabase commitTransaction] == 1+i );
		}

		// select using a record as a filter
		ParentRecord *filter = [ParentRecord record];
		filter->ID = "ID5";
		
		OOArray<ParentRecord *> sel1 = [filter select];
		assert( (int)sel1 == 1 );
		//OOPrint( sel1 );
		
		// test to-many relationship - "*" is required ///
		OOArray<ChildRecord *> sel2 = [*sel1[0] children];
		assert( (int)sel2 == 5 );
		assert( awoke == 5 );

		// check archived field
		ChildRecord *child = sel2[2];
		assert( child->strs == "ID5 ID5" );
		assert( [child parent]->ID == "ID5" );
		
		// test LIKE expression
		filter->ID = "ID%";
		sel1.fetch( filter );
		assert( (int)sel1 == 10 );
	}
	assert( rcount == 1 ); // filter is autoreleased 
	
	// web requests
	if ( allTests ) {
		assert( OOURL( "http://www.google.com" ).string() & "oogle" );
		OOString translate = OOURL( "http://translate.google.com/translate_t" ).post( "hl=en&ie=UTF-8&text=Hello&sl=en&tl=fr" );
		assert( translate & "Bonjour" );
		OOString inverse = OOURL( "http://translate.google.com/translate_t" ).post( "hl=en&ie=UTF-8&text=こんにちは&sl=ja&tl=en" );
		assert( inverse & "Hello" );
	}
	
	if ( allTests ) {
		OONode xml = *OOResource( "test.xml" ).data();
		OONode orderID = xml[@"EXAMPLE"][@"ORDER"][@"HEADER"].node();
		OONode cust = orderID[0];
		assert( cust.text()  == "0000053535" );
		cust = xml[@"EXAMPLE"][@"ORDER"][@"HEADER"]["X_ORDER_ID"];
		assert( cust.text() == "0000053535" );
		cust = xml[@"EXAMPLE"][@"ORDER"][@"HEADER"]["X_ORDER_ID"][0];
		assert( cust.text() == "0000053535" );
		assert( xml[@"EXAMPLE/ORDER/HEADER/X_ORDER_ID"] == "0000053535" );
		assert( xml[@"EXAMPLE"][@"ORDER"][@"HEADER"][@"X_ORDER_ID"] == "0000053535" );
		assert( xml[@"EXAMPLE"][@"ORDER"][@"HEADER"][@"X_ORDER_ID"][0] == "0000053535" );
		assert( xml[@"EXAMPLE"][@"ORDER"][@"HEADER"][@"X_ORDER_ID"][0] == "0000053535" );
		assert( xml[@"EXAMPLE"][@"ORDER"][@"HEADER"][@"X_ORDER_ID"].node().text() == "0000053535" );
		assert( xml[@"EXAMPLE"]["ORDER"]["@lang"] == "de" );
		assert( xml[@"EXAMPLE/ORDER/@lang"] == "de" );
		assert( xml[@"EXAMPLE/ORDER/ENTRIES/ENTRY"][0]["ENTRY_NO"] == "10" );
		assert( xml[@"EXAMPLE/ORDER/ENTRIES/ENTRY"][1]["ENTRY_NO"] == "20" );
		//OONode x = xml[@"EXAMPLE"][@"ORDER"][@"HEADER"][@"X_ORDER_ID"][0].get();
		//assert( xml[@"EXAMPLE"][@"ORDER"][@"HEADER"][@"X_ORDER_ID"][0][0] == "0000053535" );
		//assert( xml[OOPath("EXAMPLE/ORDER/HEADER/CUSTOMER_ID")][0] == "1010" );
		//assert( xml[OOPath("EXAMPLE/ORDER")]["@lang"] == "de" );
		//OONodeArray entries = xml[OOPath("EXAMPLE/ORDER/ENTRIES")].children();
		//assert( (int)entries == 2 );
		OONodeArray entries_ = xml["EXAMPLE/ORDER/ENTRIES/ENTRY"].nodes();
		assert( (int)entries_ == 2 );
		assert( (int)xml["EXAMPLE/ORDER/ENTRIES/ENTRY"].nodes() == 2 );
		OONodeArray entries0 = xml["EXAMPLE/ORDER/ENTRIES"].children();
		assert( (int)entries0 == 2 );

		assert( xml["EXAMPLE/ORDER/ENTRIES/ENTRY/1/ENTRY_NO"] == "20" );
		assert( xml["EXAMPLE/ORDER/ENTRIES/ENTRY"].node(1)["ENTRY_NO"] == "20" );
		
		OOData d1 = xml;
		OONode n1 = *d1;
		assert( xml == n1 );
		
		OONode n = OONode(), m = OONode( "m", "zzz" );
		n["a/b/c"] = "john";
#if 1
		n["a/b/e"] = "john";
		n["a/b/c/@d"] = "james";
		n["a/b/c/@a:b"] = "james";
		n["a/b/c/@xmlns"] = "http://a.b.c";
		n["a/b/c/@xmlns:x"] = "http://x.b.c";
		m["n"] = "x";
		n["a/b/c"] += m;
		m["n"] = "y";
		n["a/b/c"] += m;
		n["a/b/f"] = m;
		n["a/b/g/*"] = m;
		OOData d = n;
#endif

		//OONodeArray entries1 = xml[OOPath("EXAMPLE/ORDER/ENTRIES")][kOOChildren].get();
		//assert( (int)entries1 == 2 );
#if 1
		OONodeArray entries2 = xml[@"EXAMPLE"][@"ORDER"][@"ENTRIES"][@"ENTRY"].nodes();
		assert( (int)entries2 == 2 );
		OONodeArray entries3 = xml[@"EXAMPLE"][@"ORDER"][@"ENTRIES"][kOOChildren].get();
		assert( (int)entries3 == 2 );
#endif

#if 1
		int count = 30;300;
		NSString *iTunesUrl = OOFormat(@"http://ax.phobos.apple.com.edgesuite.net/WebObjects/MZStore.woa/wpa/MRSS/newreleases/limit=%d/rss.xml", count);
		NSData *iTunesData = &OOURL( iTunesUrl ).data();
		OONode top10 = iTunesData;
		OONodeArray items = top10["rss"]["channel"]["item"];
		assert( (int)items == count );
		OONodeArray items1 = top10["rss/channel/item"].nodes();
		assert( (int)items1 == count );

		OOData i = top10;
		OOArray<iTunesItem *>itemArray = [OOMetaData import:items intoClass:[iTunesItem class]];
#if 0
		NSLog( @"XML: %@", *xml );
		NSLog( @"YYYYYYYYYY: %@  %@ %@", *m, *n, *OOString( (const char *)[*d bytes], [*d length] ) );
		NSLog( @"ZZZZZZZZZZ: %@", *OOString( (const char *)[i bytes], [i length] ) );
		NSString *iTunesRSS = [[[NSString alloc] initWithData:iTunesData encoding:NSUTF8StringEncoding] autorelease];
		NSLog( @"DATA: %x %x %@ %@ %@", iTunesRSS, [iTunesRSS copy], iTunesRSS, @"", *top10 );
		OOPrint( itemArray );
#endif
		OONode top11 = *i;
		assert( top11 == top10 );
#endif
	}
	
	// defaults
	if ( allTests ) {
		OODefaults defaults;
		defaults[@"name"] = "value";
		defaults[@"run"] = (double)defaults[@"run"] + 1.;
	}
	
	if ( allTests ) {
		OODefaults defaults;
		assert( defaults[@"name"] == "value" );
	}

	// close/flush sqlite3 database
	[OODatabase sharedInstanceForPath:nil];

	[results setTitleWithMnemonic:@"All tests completed. OK"];
	[results setTextColor:[NSColor greenColor]];
	NSLog( @"%@", *("Tests complete "+OODefaults()[@"run"]) );
}

@end
