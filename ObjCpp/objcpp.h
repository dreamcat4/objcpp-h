/*
 *  objcpp.h - reference counting wrappers for Objective-C containers.
 *  ========
 *
 *  Created by John Holdsworth on 01/04/2009.
 *  Copyright 2009 John Holdsworth.
 *
 *  $Id: //depot/2.2/ObjCpp/objcpp.h#1 $
 *  $DateTime: 2009/11/17 13:00:06 $
 *
 *  C++ classes to wrap up XCode classes for operator overload of
 *  useful operations such as access to NSArrays and NSDictionary
 *  by subscript or NSString operators such as + for concatenation.
 *
 *  This works as the Apple Objective-C compiler supports source
 *  which mixes C++ with objective C. To enable this: for each
 *  source file which will include/import this header file, select
 *  it in Xcode and open it's "Info". To enable mixed compilation,
 *  for the file's "File Type" select: "sourcecode.cpp.objcpp".
 *
 *  For bugs or ommisions please email objcpp@johnholdsworth.com
 *
 *  Home page for updates and docs: http://objcpp.johnholdsworth.com
 *
 *  Forum for support: http://n2.nabble.com/Objcpp-Q-A-f2626793.html 
 *
 *  You may make commercial use of this source in applications without
 *  charge but not sell it as source nor can you remove this notice from
 *  this source if you redistribute. You can make any changes you like
 *  to this code before redistribution but please annotate them below.
 *
 *  If you find it useful please send a donation via paypal to account
 *  objcpp@johnholdsworth.com. Thanks.
 *
 *  THIS CODE IS PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND EITHER
 *  EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
 *  WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS
 *  THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING
 *  ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT
 *  OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
 *  TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED 
 *  BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH 
 *  ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *
 */

#include <Foundation/Foundation.h>

/************************************************************************
  Add the following to your project's "Other Sources/Project_Prefix.pch"
 ************************************************************************
 
 // precompile objcpp //
 #ifdef __cplusplus
	#import "objcpp.h"
	#import "objsql.h"
 #endif

 */

/*===========================================================================*/
/*============================== Overrides ==================================*/

/**
 For detailed debugging
 */

#ifndef OOTrace
#define OOTrace if ( _objcpp.trace ) NSLog 
#endif

/**
 Function to log warning messages
 */

#ifndef OOWarn
#ifdef OODEBUG
#define OOWarn OODump
#else
#define OOWarn NSLog
#endif
#endif

/**
 Inital value for uninitialised references. Use (id)kCFNull to detect messaging of unititialsed objects.
 */

#ifndef OOEmpty
#ifdef OOSTRICT
#define OOEmpty (id)kCFNull
#else
#define OOEmpty nil
#endif
#endif

/**
 Value to use in place of nil in NSArray and NSDictioanry. Set to nil to trap attempts to assign nil.
 */
 
#ifndef OONoValue
#ifdef OODEBUG_NOVALUE
#define OONoValue nil
#else
#define OONoValue (id)kCFNull
#endif
#endif

#ifdef OODEBUG
#define oo_inline /*  */
#else
#define oo_inline inline
#endif

/**
 Policy: Determines whether asignment from immutables automatically takes "mutableCopy".
 */

#ifndef OOCopyImmutable
#define OOCopyImmutable copy // could also be "set" or left undefined (see below)
#endif

/**
 If you ask me this is how "nil" should be defined
 */

#define OONil (id)nil
#define OONull (id)kCFNull

// containers for OOStrings
#define OOStringArray OOArray<OOString>
#define OOStringDictionary OODictionary<OOString>
#define OOStringDictionaryArray OOArray<OOStringDictionary >

#define OOStrArray OOStringArray
#define OOStringDict OOStringDictionary
#define OOStrDict OOStringDictionary
#define OOStrDicts OOStringDictionaryArray

// containers for OONumbers
#define OONumberArray OOArray<OONumber>
#define OONumberDict OODictionary<OONumber>

// for slicing arrays
inline NSRange OORange( int start, int end  ) {
	return NSMakeRange( start, end-start );
}
#define OOSlice OOArray<id>

// variations as per taste
#define OORef OOReference
#define OOPtr OOPointer
#define OOStr OOString
#define OOStrs OOStringArray
#define OOData OORef<NSData *>
#define OODate OORef<NSDate *>
#define OODict OODictionary
#define OOHash OODictionary
#define OOList OOArray
#define OOFmt OOFormat

// for debugging
static struct { BOOL trace; int retained; } _objcpp = {NO, 0};

// stack traces for debug warnings
#include <unistd.h>

static void OODump( NSString *format, ... ) {
	va_list argp; va_start(argp, format);
	NSLogv( format, argp );
	va_end( argp );

    @try {
        @throw [NSException alloc];
    }
    @catch ( NSException *ex ) {
        NSMutableString *buffer = [NSMutableString string];
		[buffer appendFormat:@"atos -p %d -printHeader", getpid()];

		for( NSNumber *n in [ex callStackReturnAddresses] )
            [buffer appendFormat:@" %u", [n intValue]];

		NSLog( @"executing:\n\n%@\n\n", buffer );
		system( [buffer UTF8String] );
		[ex release];
    }
}

#ifdef OODEBUG
#define OOPrint( _obj ) NSLog( @"%s:%d - %s = %@", __FILE__, __LINE__, #_obj, *_obj )
#else
#define OOPrint( _obj ) /**/
#endif

// forward referenced class templates
template <typename ETYPE> class OODictionary;
template <typename ETYPE> class OOArraySub;
template <typename ETYPE> class OODictionarySub;
template <typename ETYPE> class OOArraySlice;
template <typename ETYPE> class OODictionarySlice;
template <typename ETYPE,typename RTYPE,typename STYPE>
class OOSubscript;
class OOString;

/*=================================================================================*/
/*============================== Basic ref managment ========================*/

/**
 A class managing basic ref counting references retain/release mechansim for use in
 instance variables which will not allow constructors or destructors. Use the &operator
 to get a pointer with "autorelease" scope for use in the rest of your program.
 
 To free the ref either assign nil references the "=" operator or call use the ~ operator
 which returns a transient ref. Use &~var to free the ref with autorelrease scope.
 
 Usage:
 <pre>
 OOReference<NSString *> immutableRef = @"STRING"; // take referrence to a string
 OOReference<NSMutableArray *> mutableRef <<= [NSArray array]; // take mutable copy
 NSMutableArray *ptr = &amp;mutableRef; // get autoreleasing pointer to original object
 NSMutableArray *ptr = *mutableRef; // get pointer to original object
 ~mutableRef; // clear out pointer and remove "release" object.
 
 - (NSString *)function:(NSString *)str {
 OOReference<NSString *> ref = str; // take ref
 
 // do something
 
 return &amp;str; // get autoreleasing pointer to return
 // ref is discarded when object detructed on function exit.
 } 
 </pre>
 */

template <typename RTYPE>
class OOReference {
	RTYPE ref;
	oo_inline Class classOfReference() const {
		return [typeof *ref class];
	}
	// initialise ref
	oo_inline RTYPE init( RTYPE val = OOEmpty ) {
		OOTrace( @"0x%08x %s: %@", this, "INIT", val != (id)kCFNull ? (id)val : @"(NULL)" );
		ref = nil;
		return set( val );
	}
protected:
	// clear out referrence
	oo_inline void detruct() {
		OOTrace( @"0x%08x %s: 0x%08x = %@", this, "DESTUCT", ref, ref );
		set( (RTYPE)nil );
	}
	// replace referrence
	oo_inline RTYPE set( RTYPE val ) {
		if ( val && val != (id)kCFNull ) {
			[val retain];
			OOTrace( @"0x%08x %s#%d: 0x%08x = %@", this, "RETAIN", [val retainCount], val, val );
			_objcpp.retained++;
		}
		if ( ref && ref != (id)kCFNull ) {
			OOTrace( @"0x%08x %s#%d: 0x%08x = %@", this, "RELEASE", [ref retainCount], ref, ref );
			[ref release];
			_objcpp.retained--;
		}
		return ref = val;
	}
	//
	oo_inline RTYPE set( int nilOrCapacity ) {
		if ( nilOrCapacity != 0 )
			[set( [[classOfReference() alloc] initWithCapacity:nilOrCapacity] ) release];
		else
			set( (id)nil );
		return ref;
	}
	// copy any immutable objects
	oo_inline void copy( RTYPE val ) {
		RTYPE obj = [val mutableCopyWithZone:NULL];
		OOTrace( @"0x%08x %s: 0x%08x -> 0x%08x = %@", this, "COPY", val, obj, obj );
		[set( obj ) release];
	}
	oo_inline RTYPE noalloc() const {
		return ref ? ref : [[[classOfReference() alloc] init] autorelease];
	}
	// get ref with "autorelease" scope
	oo_inline RTYPE autoget() const {
		return [[get() retain] autorelease];
	}
public:
	// constructors to avoid shallow copy
	oo_inline OOReference() { init(); };
	///oo_inline OOReference( id obj ) { init( obj ); }
	oo_inline OOReference( RTYPE obj ) { init( obj ); }
	oo_inline OOReference( CFNullRef obj ) { init( (id)obj ); }
	oo_inline OOReference( const OOReference &val ) { init( val.get() ); }
	oo_inline OOReference( const OOArraySub<RTYPE> &obj ) { init( obj.get() ); }
	oo_inline OOReference( const OODictionarySub<RTYPE> &obj ) { init( obj.get() ); }
	oo_inline ~OOReference() { detruct(); }

	// allocate new ref from RTYPE specified
	oo_inline RTYPE alloc() {
		if ( !*this ) {
			RTYPE obj = [[classOfReference() alloc] init];
			OOTrace( @"0x%08x %s %@", this, "ALLOC", obj );
			[set( obj ) release];
		}
#ifndef OOCopyImmutable
#define OOCopyImmutable set
		// alternate strategy to ensure mutability
		// does not work for objects in containers
		else if ( [ref class] != classOfReference() )
			copy( ref );
#endif
		return ref;
	}
	
	// get existing ref
	oo_inline RTYPE get() const {
		return ref;
	}

	// cast operators
	oo_inline operator RTYPE () const { return get(); }
	oo_inline RTYPE operator -> () const { return get(); }
	oo_inline RTYPE operator & () const { return autoget(); }
	oo_inline RTYPE operator * () const { return !*this ? nil : get(); }
	oo_inline BOOL operator ! () const { return !ref || ref == (id)kCFNull; }

	// assign (mutable) copy from source
	oo_inline OOReference &operator <<= ( const OOReference &val ) { [set( [val.get() mutableCopy] ) release]; return *this; }
	
	// assigment by reference
	oo_inline OOReference &operator = ( const OOReference &val ) { set( val.get() ); return *this; }
	oo_inline OOReference &operator = ( CFNullRef val ) { set( val ); return *this; }
	oo_inline OOReference &operator = ( RTYPE val ) { set( val ); return *this; }
	oo_inline OOReference &operator = ( int val ) { set( val ); return *this; }

	// comparison
	oo_inline BOOL operator == ( const OOReference &val ) const { return [get() isEqual:val.get()]; }
	oo_inline BOOL operator != ( const OOReference &val ) const { return !*this == val; }
	
	/// for OOArray<id>
	oo_inline OOReference &operator += ( id val ) {
		[alloc() addObject:val];
		return *this;
	}
	
	// might be useful...
	oo_inline id operator [] ( NSString *key ) const { return [ref valueForKey:key]; }

	oo_inline OOReference operator ~ () {
		// take temporary ref and return it
		// in case &~var construct is used
		OOReference save = ref;
		detruct();
		return save;
	}

};

/*=================================================================================*/
/*=============================== Array classes ===================================*/


/**
 NSMutableArray wrapper allowing subscript syntax by index and various other operators.
 The integer value of an OArray object is the number of elements e.g. "for ( i=0 ; i<array ; i++ )".
 
 <table cellspacing=0><tr><th>operator<th>inplace<th>binary<th>argument
 <tr><td>assignment<td>&nbsp;<td>=<td>array
 <tr><td>copy<td>&nbsp;<td>&lt;&lt;=<td>array
 <tr><td>add object(s)<td>+=<td>+<td>object (or array)
 <tr><td>remove object(s)<td>-=<td>-<td>object (or array)
 <tr><td>replicate members(s)<td>*=<td>*<td>count
 <tr><td>split alternate members<td><td>/<td>count e.g 2 for even/odd members
 <tr><td>filter array<td>&=<td>&<td>array
 <tr><td>merge array<td>|=<td>|<td>array
 <tr><td>subscript<td>&nbsp;<td>[]<td>int, object or range
 </table>
 
 Usage:
 <pre>
 \@interface ExampleClass {
	OOArray array;
 }
 \@end
 
 \@implementation ExampleClass
 - (void) aFunction {
     array += @"STRING"; // append to array
     array[1] = @"STRING"; // set position directly
     NSString *str = &amp;array[1]; // take polinter to item 1
     ~array[0]; // remove element at position 0
 }

 - (NSMutableArray *)function {
 OOArray<NSString *> array; // declare array of strings
 for ( int i=0 ; i<20 )
 array[i] = @"STRING";
 return &amp;array; // return autoreleasing pointer to array allocated
 }
 
 \@end
 </pre> 
 */

template <typename ETYPE>
class OOArray : public OOReference<NSMutableArray *> {
public:
	oo_inline OOArray() {}
	oo_inline OOArray( id obj ) { *this = obj; }
	oo_inline OOArray( CFNullRef obj ) { set( (id)obj ); }
	oo_inline OOArray( const OOArray &arr ) { *this = arr; }
	oo_inline OOArray( const OOArraySub<ETYPE> &sub ) { *this = sub; }
	oo_inline OOArray( const OODictionarySub<ETYPE> &sub ) { *this = sub; }
	oo_inline OOArray( const OOSubscript<OOString,NSMutableDictionary *,OOString> &sub ) { *this = sub; }
	oo_inline OOArray( const OOArraySlice<ETYPE> &sub ) { *this = sub; }
	oo_inline OOArray( const OODictionarySlice<ETYPE> &sub ) { *this = sub; }
	oo_inline OOArray( const OOReference<NSMutableArray *> &arr ) { *this = arr; }
	oo_inline OOArray( const OODictionary<ETYPE> &dict ) { *this = dict.get(); }
	oo_inline OOArray( int nilOrCapacity ) { *this = nilOrCapacity; }
	oo_inline OOArray( NSMutableArray *arr ) { *this = arr; }
	oo_inline OOArray( NSArray *arr ) { *this = arr; }
	oo_inline OOArray( const char *val ) { *this = val; }
	oo_inline OOArray( id e1, id e2, ... ) {
		va_list argp; va_start(argp, e2);
		*this += ETYPE(e1);
		while ( e2 ) {
			*this += ETYPE(e2);
			e2 = va_arg( argp, id );
		}
		va_end( argp );
	}
	
	oo_inline NSMutableArray *operator & () const { return autoget(); }
	oo_inline operator int () const { return !*this ? 0 : [get() count]; }
	oo_inline int fetch( id parent = nil, const OOString &sql = nil );
	oo_inline OOString join( const OOString &sep = @" " ) const;
	
	oo_inline OOArray &operator = ( id val ) { set( val ); return *this; }
	oo_inline OOArray &operator = ( NSMutableArray *val ) { set( val ); return *this; }
	oo_inline OOArray &operator = ( NSArray *val ) { OOCopyImmutable( val ); return *this; }
	oo_inline OOArray &operator = ( int nilOrCapacity ) { set( nilOrCapacity ); return *this; }
	oo_inline OOArray &operator = ( const OOArray &val ) { set( val.get() ); return *this; }
	oo_inline OOArray &operator = ( const OOArraySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OOArray &operator = ( const OODictionarySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OOArray &operator = ( const OOArraySlice<ETYPE> &val ) { OOCopyImmutable( val.get() ); return *this; }
	oo_inline OOArray &operator = ( const OODictionarySlice<ETYPE> &val ) { OOCopyImmutable( val.get() ); return *this; }
	oo_inline OOArray &operator = ( const OOReference<NSMutableArray *> &val ) { set( val.get() ); return *this; }
	oo_inline OOArray &operator = ( NSMutableDictionary *val ) {
#if 0
		// objxml.h support
		if ( [val objectForKey:kOOChildren] ) {
			[alloc() setArray:[val objectForKey:kOOChildren]];
			return *this;
		}
#endif
		OOArray<id> keys = [val allKeys];
		[alloc() removeAllObjects];
		for ( int i=0 ; i<keys ; i++ ) {
			*this += (ETYPE)[*keys objectAtIndex:i];
			*this += (ETYPE)[val objectForKey:keys[i]];
		}
		return *this;
	}
	oo_inline OOArray &operator = ( const char *val );

	oo_inline BOOL operator == ( NSArray *val ) const { return [get() isEqualToArray:val]; }
	oo_inline BOOL operator != ( NSArray *val ) const { return !operator == ( val ); }
	oo_inline BOOL operator == ( NSMutableArray *val ) const { return [get() isEqualToArray:val]; }
	oo_inline BOOL operator != ( NSMutableArray *val ) const { return !operator == ( val ); }
	oo_inline BOOL operator == ( const OOArray &val ) const { return [get() isEqualToArray:val]; }
	oo_inline BOOL operator != ( const OOArray &val ) const { return !operator == ( val ); }
	
	// add elements
	oo_inline OOArray &operator += ( ETYPE val ) {
		[alloc() addObject:!val ? (ETYPE)kCFNull : val];
		return *this;
	}
	oo_inline OOArray &operator += ( const char *val ) {
		*this += (ETYPE)val;
		return *this;
	}
	oo_inline OOArray &operator += ( const OOReference<NSMutableArray *> &val ) {
		[alloc() addObjectsFromArray:val.get()];
		return *this;
	}
	
	// remove elements (not returning it)
	oo_inline OOArray &operator -= ( int sub ) {
		[alloc() removeObjectAtIndex:sub < 0 ? (int)*this+sub : sub];
		return *this;
	}
	oo_inline OOArray &operator -= ( ETYPE val ) {
		[alloc() removeObject:val];
		return *this;
	}
	oo_inline OOArray &operator -= ( const OOReference<NSMutableArray *> &val ) {
		[alloc() removeObjectsInArray:val.get()];
		return *this;
	}
	
	// replicate
	oo_inline OOArray &operator *= ( NSUInteger count ) {
		NSArray *arr = [[get() copy] autorelease];
		[get() removeAllObjects];
		for ( int i=0 ; i<count ; i++ )
			*this += arr;
		return *this;
	}
	
	oo_inline OOArray operator &= ( const OOReference<NSMutableArray *> &val ) {
		NSArray *in = [[get() copy] autorelease];
		for ( int i=[in count]-1 ; i>=0 ; i-- ) {
			id o = [in objectAtIndex:i];
			if ( !val || [val.get() indexOfObject:o] == NSNotFound )
				[get() removeObjectAtIndex:i];
		}
		return *this;
	}
	oo_inline OOArray operator |= ( const OOReference<NSMutableArray *> &val ) {
		for ( int i=0 ; i<[*val count] ; i++ ) {
			id o = [val.get() objectAtIndex:i];
			if ( [get() indexOfObject:o] == NSNotFound )
				[get() addObject:o];
		}
		return *this;
	}
	

	// binary equivalents
	oo_inline OOArray operator + ( ETYPE val ) const {
		OOArray arr; arr <<= noalloc(); arr += val; return arr;
	}
	oo_inline OOArray operator - ( ETYPE val ) const {
		OOArray arr; arr <<= noalloc(); arr -= val; return arr;
	}

	oo_inline OOArray operator + ( const OOReference<NSMutableArray *> &val ) const { 
		OOArray arr; arr <<= noalloc(); arr += val; return arr;
	}
	oo_inline OOArray operator - ( const OOReference<NSMutableArray *> &val ) const { 
		OOArray arr; arr <<= noalloc(); arr -= val; return arr;
	}
	oo_inline OOArray operator & ( const OOReference<NSMutableArray *> &val ) const { 
		OOArray arr; arr <<= noalloc(); arr &= val; return arr;
	}
	oo_inline OOArray operator | ( const OOReference<NSMutableArray *> &val ) const {
		OOArray arr; arr <<= noalloc(); arr |= val; return arr;
	}	

	// subscript into array
	oo_inline OOArraySub<ETYPE> operator [] ( int sub ) const  {
		return OOArraySub<ETYPE>( this, sub );
	}
	oo_inline OOArraySub<ETYPE> operator [] ( ETYPE sub ) const {
		return (*this)[[get() indexOfObject:sub]];
	}
	oo_inline OOArraySlice<ETYPE> operator [] ( const NSRange &subs ) const {
		return OOArraySlice<ETYPE>( this, subs );
	}
	
	// sort array (of strings)
	oo_inline OOStringArray operator + () const {
		return [get() sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
	}	
	
	// array in reverse order
	oo_inline OOArray operator - () const {
		return [[get() reverseObjectEnumerator] allObjects];
	}

	// shift first element
	oo_inline ETYPE operator -- () {
		return ~(*this)[0];
	}
	
	// pop last element
	oo_inline ETYPE operator -- ( int ) {
		return ~(*this)[-1];
	}
};

template <typename ETYPE>
oo_inline OOArray<ETYPE> operator * ( const OOArray<ETYPE> &left, int count ) {
	OOArray<ETYPE> out( (int)left*count );
	for ( int i=0 ; i<count ; i++ ) 
		out += left;
	return out;
}

template <typename ETYPE>
oo_inline OOArray<ETYPE> operator / ( const OOArray<ETYPE> &left, int count ) {
	OOArray<ETYPE> out( count );
	for ( int i=0 ; i<left ; i++ ) 
		out[i%count][i/count] = left[i];
	return out;
}


/*=================================================================================*/
/*============================== Dictionary classes ===============================*/

/**
 NSMutableDictionary wrapper for use in instance variables which allows subscripting
 by the key value. Use &~dict[@"key"] to remove the entry and return it with expression
 or autorelease scope. Subscripting can be applied recusively. ETYPE is type of leaf node.
 
 Operators:
 <table cellspacing=0><tr><th>operator<th>inplace<th>arguments
 <tr><td>take referernce<td>=<td>dictionary
 <tr><td>take copy<td>&lt;&lt;=<td>dictionary
 <tr><td>merge entries<td>+=<td>dictionary
 <tr><td>remove entries<td>-=<td>key or array
 <tr><td>sbuscript<td>[]<td>object (key) or "slice" of keys
 </table>
 
 Usage:
 <pre>

 - (void)function:(OODictionary<NSString *> &)dict {
	dict[@"ONE"][@"TWO"][@"THREE"] = @"Four"; // set valule
    NSString *str = *dict[@"ONE"][@"TWO"][@"THREE"]; // get value
    ~dict[@"ONE"][@"TWO"][@"THREE"]; // delete value
    // pointer "str" is invalid at this point
 }
 
 - (void)function:(NSMutableDictionary *)dict {
 OODictionary<OOString> ref <<= dict; // initialise mutable copy
 ref[@"KEY"] <<= @"";
 for ( int i=0 ; i<10 ; i++ )
 ref[@"KEY"] += "ABC";
 }
 
 </pre>
 */

template <typename ETYPE>
class OODictionary : public OOReference<NSMutableDictionary *> {
public:
	oo_inline OODictionary() {}
	oo_inline OODictionary( id obj ) { *this = obj; }
	oo_inline OODictionary( CFNullRef obj ) { set( (id)obj ); }
	oo_inline OODictionary( const OODictionary &dict ) { *this = dict; }
	oo_inline OODictionary( const OOArray<ETYPE> &arr ) { *this = arr.get(); }
	oo_inline OODictionary( const OOArraySub<ETYPE> &sub ) { *this = sub; }
	oo_inline OODictionary( const OODictionarySub<ETYPE> &sub ) { *this = sub; }
	oo_inline OODictionary( const OOReference<NSMutableDictionary *> &val ) { set( val.get() ); }
	oo_inline OODictionary( int nilOrCapacity ) { *this = nilOrCapacity; }
	oo_inline OODictionary( NSMutableDictionary *dict ) { *this = dict; }
	oo_inline OODictionary( NSDictionary *dict ) { *this = dict; }
	oo_inline OODictionary( const char *val ) { *this = val; }
	oo_inline OODictionary( id e1, id e2, ... ) {
		va_list argp; va_start(argp, e2);
		do 
			(*this)[e1] = ETYPE(e2);
		while ( (e1 = va_arg( argp, id )) && (e2 = va_arg( argp, id )) );
		va_end( argp );
	}

	oo_inline NSMutableDictionary *operator & () const { return autoget(); }

	oo_inline OODictionary &operator = ( id val ) { set( val ); return *this; }
	oo_inline OODictionary &operator = ( NSMutableDictionary *val ) { set( val ); return *this; }
	oo_inline OODictionary &operator = ( NSDictionary *val ) { OOCopyImmutable( val ); return *this; }
	oo_inline OODictionary &operator = ( int nilOrCapacity ) { set( nilOrCapacity ); return *this; }
	oo_inline OODictionary &operator = ( const OODictionary &val ) { set( val.get() ); return *this; }
	oo_inline OODictionary &operator = ( const OOArraySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OODictionary &operator = ( const OODictionarySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OODictionary &operator = ( const OOReference<NSMutableDictionary *> &val ) { set( val.get() ); return *this; }

	oo_inline OODictionary &operator = ( NSMutableArray *val ) {
		OOArray<id> keys = val;
		OOArray<ETYPE> values = val;
		[alloc() removeAllObjects];
		for ( int i=0 ; i<keys ; i+=2 )
#if 00000
			(*this)[*keys[i]] = *values[i+1]; /////
#else
			(*this)[[*keys objectAtIndex:i]] = *values[i+1]; /////
#endif
		return *this;
	}
	oo_inline OODictionary &operator = ( const char *val );

	oo_inline BOOL operator == ( NSDictionary *val ) const { return [get() isEqualToDictionary:val]; }
	oo_inline BOOL operator != ( NSDictionary *val ) const { return !operator == ( val ); }
	oo_inline BOOL operator == ( NSMutableDictionary *val ) const { return [get() isEqualToDictionary:val]; }
	oo_inline BOOL operator != ( NSMutableDictionary *val ) const { return !operator == ( val ); }
	oo_inline BOOL operator == ( const OODictionary &val ) const { return [get() isEqualToDictionary:val]; }
	oo_inline BOOL operator != ( const OODictionary &val ) const { return !operator == ( val ); }

	// dctionary operations
	oo_inline OODictionary &operator -= ( id val ) { 
		[alloc() removeObjectForKey:val]; 
		return *this;
	}
	oo_inline OODictionary &operator -= ( const OOReference<NSMutableArray *> &val ) {
		[alloc() removeObjectsForKeys:val]; 
		return *this;
	}
	oo_inline OODictionary &operator -= ( const OOReference<NSMutableDictionary *> &val ) {
		[alloc() removeObjectsForKeys:[val allKeys]]; 
		return *this;
	}
	
	oo_inline OODictionary &operator &= ( const OODictionary &val ) {
#if 00000
		for ( id key in [get() allKeys] )
			if ( !val[key] )
				*this -= key;
		return *this;
#else
		OOArray<id> keys = [get() allKeys];
		for ( int i=0 ; i<keys ; i++ )
			if ( !val[keys[i]] )
				*this -= keys[i];
		return *this;
#endif
	}
	oo_inline OODictionary &operator |= ( const OODictionary &val ) {
		OOArray<id> keys = [val.get() allKeys];
		for ( int i=0 ; i<keys ; i++ )
			if ( !(*this)[keys[i]] )
				(*this)[keys[i]] = val[keys[i]];
		return *this;
	}
		
	oo_inline OODictionary operator & ( const OODictionary &val ) const { 
		OODictionary out; out <<= noalloc(); return out &= val;
	}
	oo_inline OODictionary operator | ( const OODictionary &val ) const {
		OODictionary out; out <<= noalloc(); return out |= val;
	}
	
	oo_inline OODictionarySub<ETYPE> operator [] ( id sub ) const {
		return OODictionarySub<ETYPE>( this, sub );
	}
	oo_inline OODictionarySub<ETYPE> operator [] ( const char *sub ) const {
		return (*this)[[NSString stringWithUTF8String:sub]];
	}
	oo_inline OODictionarySub<ETYPE> operator [] ( const CFStringRef sub ) const {
		return (*this)[(id)sub];
	}	
	oo_inline OODictionarySub<ETYPE> operator [] ( const OOString &sub ) const {
		return OODictionarySub<ETYPE>( this, sub );
	}
	oo_inline OODictionarySub<ETYPE> operator [] ( const OOArraySub<id> &sub ) const {
		return OODictionarySub<ETYPE>( this, sub );
	}
	oo_inline OODictionarySub<ETYPE> operator [] ( const OOArraySub<OOString> &sub ) const;
	oo_inline OODictionarySub<ETYPE> operator [] ( const OODictionarySub<OOString> &sub ) const;
	
	oo_inline OODictionarySlice<ETYPE> operator [] ( const OOReference<NSMutableArray *> &subs ) const {
		return OODictionarySlice<ETYPE>( this, subs.get() );
	}
};

/**
 Internal abstract superclass for subscripting operations by operator []
 */

template <typename ETYPE,typename RTYPE,typename STYPE>
class OOSubscript {
	friend class OONodeSub;

protected:
	// for x = int, NSString *, NSRange, OOArray<id>
	const OOReference<RTYPE *> *root; // for simple var[x]
	OOArraySub<RTYPE *> *aref; // for var...[0][x]
	OODictionarySub<RTYPE *> *dref; // for var...[@"KEY"][x]

	oo_inline OOSubscript() {
		root = NULL; aref = NULL; dref = NULL;
		references = 0;
	}
	oo_inline ~OOSubscript () {
		if ( aref && --aref->references == 0 )
			delete aref;
		if ( dref && --dref->references == 0 )
			delete dref;
	}

	oo_inline id autoget() const {
		return [[get() retain] autorelease];
	}

	oo_inline virtual id get( BOOL warn = YES ) const { return nil; }
	oo_inline virtual id set( id val ) const { return nil; }

public:
	int references;
	
	oo_inline id alloc( Class c ) const {
		id parent = get( NO );
		if ( !parent ) {
			parent = [[c alloc] init];
			OOTrace( @"0x%08x %s %@", this, "VIVIFY", parent );
			[set( parent ) release];
		}
#if 001
		else if ( ![parent isKindOfClass:c] ) {
			OOWarn( @"Reset: %@ == %@", [parent class], c );
			[set( parent = [parent mutableCopy] ) release];
		}
#endif
		return parent;
	}
	
	RTYPE *parent( BOOL allocate ) const {
		if ( allocate )
			return root ? ((OOReference<RTYPE *> *)root)->alloc() : aref ? aref->alloc( [RTYPE class] ) : dref->alloc( [RTYPE class] );
		else
			return root ? root->get() : aref ? aref->get( NO ) : dref->get( NO );
	}
	
	// unaries	
	oo_inline operator ETYPE () const { return get(); }
	////oo_inline operator NSString * () const { return get(); }
	oo_inline ETYPE operator * () const { return get(); }
	oo_inline ETYPE operator -> () const { return get(); }
	oo_inline id operator & () const { return autoget(); }
	oo_inline BOOL operator ! () const { return !get( NO ) || get( NO ) == (id)kCFNull; }
	///oo_inline operator BOOL () const { return !!*this; }

	// recursive subscripting
	oo_inline OOArraySub<STYPE> operator [] ( int sub ) const {
		return OOArraySub<STYPE>( (OODictionarySub<NSMutableArray *> *)this, sub );
	}
	oo_inline OOArraySlice<STYPE> operator [] ( const NSRange &sub ) const {
		return OOArraySlice<STYPE>( (OODictionarySub<NSMutableArray *> *)this, sub );
	}
	oo_inline OODictionarySub<STYPE> operator [] ( id sub ) const {
		return OODictionarySub<STYPE>( (OODictionarySub<NSMutableDictionary *> *)this, sub );
	}
	oo_inline OODictionarySub<STYPE> operator [] ( const char *sub ) const {
		return (*this)[[NSString stringWithUTF8String:sub]];
	}	
	oo_inline OODictionarySub<STYPE> operator [] ( const CFStringRef sub ) const {
		return (*this)[(id)sub];
	}	
	oo_inline OODictionarySlice<STYPE> operator [] ( const OOReference<NSMutableArray *> &sub ) const {
		return OODictionarySlice<STYPE>( (OODictionarySub<NSMutableDictionary *> *)this, sub );
	}
	
	// asignment
	oo_inline OOSubscript &operator <<= ( id<NSMutableCopying> val ) {
 		[set( [val mutableCopyWithZone:NULL] ) release]; 
		return *this;
	}
	oo_inline OOSubscript &operator <<= ( const char *val ) {
 		set( OOString( val ) );
		return *this;
	}

	oo_inline OOSubscript &operator = ( NSMutableString *val ) { set( val ); return *this; }
	oo_inline OOSubscript &operator = ( NSString *val ) { *this <<= val; return *this; }
	oo_inline OOSubscript &operator = ( NSNumber *val ) { set( val ); return *this; }
	oo_inline OOSubscript &operator = ( NSNull *val ) { set( val ); return *this; }
	
	// comparison
	oo_inline BOOL operator == ( const ETYPE val ) const { return **this == val; }
	oo_inline BOOL operator != ( const ETYPE val ) const { return **this != val; }
	oo_inline BOOL operator >= ( const ETYPE val ) const { return **this >= val; }
	oo_inline BOOL operator <= ( const ETYPE val ) const { return **this <= val; }
	oo_inline BOOL operator >  ( const ETYPE val ) const { return **this >  val; }
	oo_inline BOOL operator <  ( const ETYPE val ) const { return **this <  val; }
	
	// inplace operators
	oo_inline OOSubscript &operator += ( const ETYPE val ) { return *this = **this + val; }
	oo_inline OOSubscript &operator -= ( const ETYPE val ) { return *this = **this - val; }
	oo_inline OOSubscript &operator *= ( const ETYPE val ) { return *this = **this * val; }
	oo_inline OOSubscript &operator /= ( const ETYPE val ) { return *this = **this / val; }
	oo_inline OOSubscript &operator %= ( const ETYPE val ) { return *this = **this % val; }
	oo_inline OOSubscript &operator &= ( const ETYPE val ) { return *this = **this & val; }
	oo_inline OOSubscript &operator |= ( const ETYPE val ) { return *this = **this | val; }
	
	// binary operators
	oo_inline ETYPE operator + ( const ETYPE val ) const { return **this + val; }
	oo_inline ETYPE operator - ( const ETYPE val ) const { return **this - val; }
	oo_inline ETYPE operator * ( const ETYPE val ) const { return **this * val; }
	oo_inline ETYPE operator / ( const ETYPE val ) const { return **this / val; }
	oo_inline ETYPE operator % ( const ETYPE val ) const { return **this % val; }
	oo_inline ETYPE operator & ( const ETYPE val ) const { return **this & val; }
	oo_inline ETYPE operator | ( const ETYPE val ) const { return **this | val; }

	// concatenate others
	oo_inline ETYPE operator + ( NSString *val ) { return **this + val; }
#if 0000
	oo_inline ETYPE operator + ( const char *val ) { return **this + val; }
	oo_inline ETYPE operator + ( double val ) { return **this + val; }
	oo_inline ETYPE operator + ( int val ) { return **this + val; }
	oo_inline ETYPE operator + ( id val ) { return **this + val; }
#endif
};

/**
 Internal class to represent a subscript operation in an expression so it can
 be assigned to. You can also use the ~val[i] operation to remove the value
 from the array and return it with either expression or autorelease scope.
 If the index assigned to is beyond the end of the array it will be padded
 with kCFNull values to allow for sparse arrays.
 
 Usage:
<pre>
 - (AVAudioPlayer *)play:OOArray<AVAudioPlayer *> &amp;sounds {
 [*sounds[0] play]; // use * operator to get actual object ref
 return &amp;~sounds[0]; // delete item at index 0 and return pointer
 }
</pre>
 */

template <typename ETYPE>
class OOArraySub : public OOSubscript<ETYPE,NSMutableArray,ETYPE> {
	friend class OOArray<ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableArray,ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableDictionary,ETYPE>;
	friend class OOSubscript<OOArray<ETYPE>,NSMutableArray,ETYPE>;
	friend class OOSubscript<OOArray<ETYPE>,NSMutableDictionary,ETYPE>;
	friend class OONodeArraySub;

	int idx;

	oo_inline OOArraySub( const OOArray<ETYPE> *ref, int sub ) {
		this->root = ref;
		idx = sub < 0 ? [this->parent( NO ) count]+sub : sub;
	}
	oo_inline OOArraySub( OOArraySub<NSMutableArray *> *ref, int sub ) {
		this->aref = ref;
		idx = sub < 0 ? [this->parent( NO ) count]+sub : sub;
	}
	oo_inline OOArraySub( OODictionarySub<NSMutableArray *> *ref, int sub ) {
		this->dref = ref;
		idx = sub < 0 ? [this->parent( NO ) count]+sub : sub;
	}

public:
	oo_inline virtual id get( BOOL warn = YES ) const {
		NSMutableArray *arr = this->parent( NO );
		id ret = nil;
		if ( arr != (id)kCFNull )
			if ( idx < 0 )
				OOWarn( @"0x%08x Excess negative index (%d) beyond size of array (%d)", this, idx-[arr count], [arr count] );
			else if ( idx < [arr count] )
				ret = [arr objectAtIndex:idx];
			else if ( warn )
				OOWarn( @"0x%08x Array ref (%d) beyond end of array (%d)", this, idx, [arr count] );
		return ret;
	}
	oo_inline virtual id set ( id val ) const {
		NSMutableArray *arr = this->parent( YES );
		NSUInteger count = [arr count];
		if ( val == nil )
			val = OONoValue;
		if ( idx < count )
			[arr replaceObjectAtIndex:idx withObject:val];
		else {
			while ( count++ < idx )
				[arr addObject:(id)kCFNull]; // padding for sparse arrays
			[arr addObject:val];
		}
		return val;
	}
	
	oo_inline operator NSUInteger () const { return idx; }

	// assign and assign by mutable copy 
	oo_inline OOArraySub &operator = ( ETYPE val ) { set( val ); return *this; }
	oo_inline OOArraySub &operator = ( const char *val ) { *this = OOString(val)/*.get()*/; return *this; }
	oo_inline OOArraySub &operator = ( const OOArraySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OOArraySub &operator = ( const OODictionarySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OOArraySub &operator = ( const OOArraySlice<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OOArraySub &operator = ( const OODictionarySlice<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OOArraySub &operator = ( const OOReference<NSMutableArray *> &val ) { set( val.get() ); return *this; }
	oo_inline OOArraySub &operator = ( const OOReference<NSMutableDictionary *> &val ) { set( val.get() ); return *this; }

	// delete element and return it
	oo_inline ETYPE operator ~ () {
		id obj = get();
		int rc = [obj retainCount];
		ETYPE save = obj;
		if ( rc == [obj retainCount] )
			this->autoget();
		NSMutableArray *arr = this->parent( NO );
		if ( 0 <= idx && idx < [arr count] )
			[arr removeObjectAtIndex:idx];
		else
			OOWarn( @"0x%08x Attempt to remove index (%d) beyond end of array (%d)", this, idx, [arr count] );
		return save;
	}
};

/**
 Internal class representing subscript by key in an expression so it 
 can be assigned to. Subscripts can be applied recursively "viviifying"
 the required Dictionaries (or arrays) at each node as required. Array
 and Dictionary acces can be mixed recursively.
 
 Usage:
 <pre>
 OODictionary<id> dict;
 dict[@"DICT"]["KEY"] <<= @"TEXT1";
 dict[@"ARRAY"][0] = <<= @"TEXT2";
 </pre>
 */

template <typename ETYPE>
class OODictionarySub : public OOSubscript<ETYPE,NSMutableDictionary,ETYPE> {
	friend class OODictionary<ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableArray,ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableDictionary,ETYPE>;
	friend class OODefaultsSub;
	friend class OONodeArraySub;
	friend class OONodeSub;
	friend class OONode;
	
	id key;

	oo_inline OODictionarySub( const OOReference<NSMutableDictionary *> *ref, id sub ) {
		this->root = ref; this->key = sub;
	}
	oo_inline OODictionarySub( OOArraySub<NSMutableDictionary *> *ref, id sub ) {
		this->aref = ref; this->key = sub;
	}
	oo_inline OODictionarySub( OODictionarySub<NSMutableDictionary *> *ref, id sub ) {
		this->dref = ref; this->key = sub;
	}

public:
	oo_inline virtual id get( BOOL warn = YES ) const {
		id parent = this->parent( NO );
		return parent != (id)kCFNull ? [parent objectForKey:this->key] : nil;
	}
	oo_inline virtual id set( id val ) const {
		if ( val == nil )
			val = OONoValue;
		[this->parent( YES ) setObject:val forKey:this->key];
		return val;
	}

	// assign and assign by mutable copy 
	oo_inline OODictionarySub &operator = ( ETYPE val ) { set( val ); return *this; }
	oo_inline OODictionarySub &operator = ( const char *val ) { *this = OOString(val)/*.get()*/; return *this; }
	oo_inline OODictionarySub &operator = ( const OOArraySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OODictionarySub &operator = ( const OODictionarySub<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OODictionarySub &operator = ( const OOArraySlice<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OODictionarySub &operator = ( const OODictionarySlice<ETYPE> &val ) { set( val.get() ); return *this; }
	oo_inline OODictionarySub &operator = ( const OOReference<NSMutableArray *> &val ) { set( val.get() ); return *this; }
	oo_inline OODictionarySub &operator = ( const OOReference<NSMutableDictionary *> &val ) { set( val.get() ); return *this; }
	
	// delete entry and return its value
	oo_inline ETYPE operator ~ () {
		id obj = get();
		int rc = [obj retainCount];
		ETYPE save = obj;
		if ( rc == [obj retainCount] )
			this->autoget();
		[this->parent( NO ) removeObjectForKey:this->key];
		return save;
	}
};

/**
 Class representing taking a sub array of objects from an array references a NSRange
 and the subscript to the operator []. OORange takes arguments start, end
 whereas NSMakeRange takes argument start, length/count. Either can be used.
 
 Usage:
 <pre>
 OOStringArray strings = "a b c d";
 if ( strings[NSMakeRange(1,2)] == "a b" )
    ;// should be true
 strings[OORange(1,3)] = OOStringArray( "x y" );
 if ( strings == "a x y d" )
    ; // should be true
 </pre>
 */

template<typename ETYPE>
class OOArraySlice : public OOSubscript<OOArray<ETYPE>,NSMutableArray,ETYPE> {
	friend class OOArray<ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableArray,ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableDictionary,ETYPE>;
	friend class OOSubscript<OOArray<ETYPE>,NSMutableArray,ETYPE>;
	friend class OOSubscript<OOArray<ETYPE>,NSMutableDictionary,ETYPE>;

	NSRange slice;

	oo_inline void setSlice( const NSRange &sub ) {
		if ( (slice = sub).location < 0 )
			slice.location = [this->parent( NO ) count]+slice.location;
	}
	
	oo_inline OOArraySlice( const OOArray<ETYPE> *ref, const NSRange &sub ) {
		this->root = ref; setSlice( sub );
	}
	oo_inline OOArraySlice( OOArraySub<NSMutableArray *> *ref, const NSRange &sub ) {
		this->aref = ref; setSlice( sub );
	}
	oo_inline OOArraySlice( OODictionarySub<NSMutableArray *> *ref, const NSRange &sub ) {
		this->dref = ref; setSlice( sub );
	}
public:
	oo_inline virtual id get( BOOL warn = YES ) const {
		return [this->parent( NO ) subarrayWithRange:slice];
	}
	oo_inline BOOL operator == ( const OOArray<ETYPE> &in ) const {
		return [get() isEqualToArray:in.get()];
	}
	oo_inline OOArraySlice &operator = ( const OOArray<ETYPE> &in ) {
		NSMutableArray *parent = this->parent( YES );
		[parent removeObjectsInRange:slice];
		for ( int i=0 ; i<in ; i++ )
			[parent insertObject:*in[i] atIndex:slice.location+i];
		return *this;
	}
	oo_inline OOArray<ETYPE> operator ~ () {
		OOArray<ETYPE> save = get();
		[this->parent( NO ) removeObjectsInRange:slice];
		return save;
	}
};

/**
 Class representing taking an array of objects from a dictionary references a
 "slice" of keys. Can be assigned to.
 
 Usage:
 <pre>
 OOStringDict dict = "a 1 b 2 c 3 d 4";
 if ( dict[OOSlice("b c")] == "2 3" )
   ;// should be true
 dict[OOSlice("b c")] = OOStringArray( "9 9" );
 if ( dict[OOSlice("b c")] = "9 9" )
   ; // should be true
 </pre>
 */

template<typename ETYPE>
class OODictionarySlice : public OOSubscript<OOArray<ETYPE>,NSMutableDictionary,ETYPE> {
	friend class OODictionary<ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableArray,ETYPE>;
	friend class OOSubscript<ETYPE,NSMutableDictionary,ETYPE>;

	OOArray<id> slice;

	oo_inline OODictionarySlice( const OODictionary<ETYPE> *ref, NSMutableArray *sub ) {
		this->root = ref; slice = sub;
	}
	oo_inline OODictionarySlice( OOArraySub<NSMutableDictionary *> *ref, NSMutableArray *sub ) {
		this->aref = ref; slice = sub;
	}
	oo_inline OODictionarySlice( OODictionarySub<NSMutableDictionary *> *ref, NSMutableArray *sub ) {
		this->dref = ref; slice = sub;
	}
public:
	oo_inline virtual id get( BOOL warn = YES ) const {
		return [this->parent( NO ) objectsForKeys:slice notFoundMarker:(id)kCFNull];
	}
	oo_inline BOOL operator == ( const OOArray<ETYPE> &in ) const {
		return [get() isEqualToArray:in.get()];
	}
	oo_inline OODictionarySlice &operator = ( const OOArray<ETYPE> &in ) {
		if ( (int)in != (int)slice )
			OOWarn( @"Slice assignment with key count [%d] different to value count [%d] - %@ c.f. %@", 
				   (int)slice, (int)in, *slice, *in );
		for ( int i=0 ; i<in && i<slice ; i++ )
			[this->parent( YES ) setObject:*in[i] forKey:slice[i]];
		return *this;
	}	
	oo_inline OOArray<ETYPE> operator ~ () {
		OOArray<ETYPE> save = get();
		[this->parent( NO ) removeObjectsForKeys:slice];
		return save;
	}
};

#import "objstr.h"
