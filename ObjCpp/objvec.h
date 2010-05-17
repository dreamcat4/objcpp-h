/*
 *  objvec.h - varaiable length storage classes for Objective-C
 *  ========
 *
 *  Created by John Holdsworth on 01/04/2009.
 *  Copyright 2009 John Holdsworth.
 *
 *  $Id: //depot/2.2/ObjCpp/objvec.h#1 $
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

#import "objcpp.h"

static struct { int buffered; } _objvec;

/*====================================================================================*/
/*============================== Variable length buffer===============================*/

/**
 Internal class to allocate storage on demand as it is accessed by ref using the
 subscript operator. A memory is allocated and reallocated if the element type is a
 plain type it will be zeroed out otherwise if it is a class the constructor will
 be called for each element in the array as it is allocated. On reallocation a new
 oversized array is allocated then the "=" operator used to copy the old values
 to the new array. This operator should "steal" any deep _refs from the old
 instance before it is detructed as does the OOBuffer class itself as it is used
 recursively.
 
 Usage:
 </pre>
 OOBuffer<OOBuffer<double> > x; // not ref counted however
 // and can not be put into arrays or dictionaries
 x[99][99] = 43;
 </pre>
 */

template <typename ETYPE>
class OOBuffer {
	ETYPE *buff;
	void realloc( int size ) {
		ETYPE *next = new ETYPE[size];
		const char *info = @encode(ETYPE);
		if ( info[0] != '{' )
			memset( next, '\000', size * sizeof *next );
		OOTrace( @"0x%08x %s [%d] 0x%08x = 0x%08x %s", this, "BREALLOC", size, buff, next, info );
		if ( buff ) {
			for ( int i=0 ; i<allocated ; i++ )
				next[i] = buff[i];
			delete[] buff;
		}
		allocated = size;
		buff = next;
	}
	
public:
	int used, allocated, bounds;
	
	oo_inline OOBuffer() {
		_objvec.buffered++;
		buff = NULL;
		used = allocated = bounds = 0;
		OOTrace( @"0x%08x %s", this, "BCONSTRUCT" );
	}
	oo_inline OOBuffer( int size ) {
		_objvec.buffered++;
		buff = NULL;
		realloc( bounds = size );
		used = 0;
		OOTrace( @"0x%08x %s [%d] 0x%08x", this, "BCONSTRUCT", size, buff );
	}
	oo_inline ETYPE &operator [] ( int sub ) {
		// reallocate?
		if ( !buff || sub+1 > allocated )
			realloc( (sub+11)*4/3 );
		
		// update elements used
		if ( used < sub+1 )
			used = sub+1;
		else if ( sub < 0 )
			sub += bounds ? bounds : used;
		else if ( bounds && sub+1 > bounds )
			OOWarn( @"0x%08x Subscript [%d] beyond bounds of array (%d)", this, sub, bounds );
		return buff[sub];
	}
	
	// steal pointer to contents when copied on reallocate of parent
	oo_inline OOBuffer &operator = ( OOBuffer & from ) {
		OOTrace( @"0x%08x %s, 0x%08x = 0x%08x", this, "BASSIGN", buff, from.buff );
		buff = from.buff;
		used = from.used;
		allocated = from.allocated;
		bounds = from.bounds;
		from.buff = NULL;
		return *this;
	}
	oo_inline ~OOBuffer() {
		_objvec.buffered--;
		if( buff ) {
			OOTrace( @"0x%08x %s 0x%08x", this, "BDESTRUCT", buff );
			delete[] buff;
		}
	}
};

/*====================================================================================*/
/*============================== Pointer classes =====================================*/

/**
 OOPointer subclass to store a pointer to a heap allocated C++ class instance
 inside an NSValue object to it can be ref counted and stored in OSX
 buffs such as NSMutableDictionary.
 
 Usage:
 <pre>
 OOClassPtr<aclass> avar = new aclass();
 OODictionary<OOClassPtr<aclass>> a dict;
 dict[@"KEY"] = avar;
 ~avar;
 ~dict[@"KEY"]; // instance deleted here
 </pre>
 */

template <typename CNAME>
class OOClassPtr : public OOPointer<CNAME *> {
protected:
	oo_inline NSValue *cset( NSValue *val ) {
		if ( !!*this && val != this->get() )
			cdetruct();
		return this->pset( val );
	}
	oo_inline CNAME &cset( CNAME *cptr ) {
		if ( !!*this && cptr != this->pget() )
			cdetruct();
		return *pset( cptr );
	}
	oo_inline CNAME &calloc() {
		if ( !this->pget() ) {
			OOTrace( @"0x%08x %s", this, "CALLOC" );
			cset( new CNAME() );
		}
		return *this->pget();
	}
	oo_inline void cdetruct() {
		if ( this->pget() && [this->get() retainCount] == 1 ) {
			OOTrace( @"0x%08x %s", this, "CDESTOY" );
			delete this->pget();
		}
		this->detruct();
	}
public:
	oo_inline OOClassPtr() {}
	oo_inline OOClassPtr( CNAME *cptr ) { *this = cptr; }
	oo_inline OOClassPtr( CNAME &cptr ) { *this = &cptr; }
	oo_inline OOClassPtr( NSValue *val ) { *this = val; }
	oo_inline OOClassPtr( const OOClassPtr &val ) { *this = val.get(); }
	oo_inline ~OOClassPtr() { cdetruct(); }
	
	oo_inline OOClassPtr &operator = ( CNAME *cptr ) { cset( cptr ); return *this;	}
	oo_inline OOClassPtr &operator = ( CNAME &cptr ) { cset( &cptr ); return *this;	}
	oo_inline OOClassPtr &operator = ( NSValue *val ) { cset( val ); return *this; }
	oo_inline OOClassPtr &operator = ( const OOClassPtr &val ) { cset( val.get() ); return *this; }
	
	oo_inline CNAME &cget() { return calloc(); } 
	oo_inline CNAME &operator * () { return calloc(); } 		
	
	oo_inline OOClassPtr<CNAME> operator ~ () {
		OOClassPtr<CNAME> save = *this;
		this->detruct();
		return save;
	}	
};

/*===================================================================================*/
/*=============================== Storage classes ===================================*/

/**
 A more efficient buff class than NSArray for any element type (int, id, OOReference<NSString> etc.).
 Memory is allocated on deamnd when _refd by the subscipt operator and if the type is a class
 its constructors and destructors will be called by the underlying OOBuffer class.
 
 Usage:
 <pre>
 OOVector<double> array;
 array[99] = 43.
 OOVector<OOString> strarry;
 array[99] = @"THIS";
 </pre>
 */

template <typename ETYPE>
class OOVector : public OOClassPtr< OOBuffer<ETYPE> > {
public:
	oo_inline OOVector() { }
	oo_inline OOVector( const OOVector &val ) { *this = val; }
	///oo_inline OOVector( OOClassPtr<OOVector<ETYPE> > &val ) { *this = val.cget(); }
	oo_inline OOVector( int size ) { cset( new OOBuffer<ETYPE>( size ) ); }
	oo_inline OOVector( const OOArray<ETYPE> &array ) { 
		int size = (int)array;
		cset( new OOBuffer<ETYPE>( size ) ); 
		for ( int i=0 ; i<size ; i++ )
			(*this)[i] = array[i];
	}

	oo_inline OOVector &operator = ( NSValue *val ) { this->cset( val ); return *this; }
	oo_inline OOVector &operator = ( const OOVector &val ) { cset( val.get() ); return *this; }

	oo_inline operator int () { 
		return this->cget().used;
	}
	oo_inline ETYPE &operator [] ( int sub ) {
		return this->cget()[ sub ];
	}
	oo_inline OOVector &operator += ( ETYPE value ) {
		(*this)[this->cget().used] = value;
		return *this;
	}
};

/**
 A two dimensional Container class of any size using in effect a OOVector of OOBuffer instances.
 */

template <typename ETYPE>
class OOMatrix : public OOVector< OOBuffer<ETYPE> > {
public:
	oo_inline OOMatrix() { }
	oo_inline OOMatrix( const OOMatrix &val ) { *this = val; }
	
	oo_inline OOMatrix &operator = ( NSValue *val ) { this->cset( val ); return *this; }
	oo_inline OOMatrix &operator = ( const OOMatrix<ETYPE> &val ) { cset( val.get() ); return *this; }
};

/**
 A three dimensional Container class of any size using a OOVector of OOBuffers of OOBuffers.
 */

template <typename ETYPE>
class OOCube : public OOVector< OOBuffer< OOBuffer<ETYPE> > > {
public:
	oo_inline OOCube() { }
	oo_inline OOCube( const OOCube &val ) { *this = val; }
	oo_inline OOCube &operator = ( NSValue *val ) { this->cset( val ); return *this; }
	oo_inline OOCube &operator = ( const OOCube &val ) { cset( val.get() ); return *this; }
};

/**
 A faster NSMutableArray replacement that correctly ref counts and releases Objective-C objects.
 
 Usage:
 <pre>
 OOObjects<NSString *> var;
 var[99] = @"THIS";
 </pre>
 */

template <typename RTYPE>
class OOObjects : public OOVector< OOReference<RTYPE> > {
public:
	oo_inline OOObjects() { }
	oo_inline OOObjects( int size ) { cset(  new OOBuffer< OOReference<RTYPE> >( size ) ); }
	oo_inline OOObjects( const OOArray<RTYPE> array ) { *this = array.get(); }
	oo_inline OOObjects( NSMutableArray *arr ) { *this = arr; }
	oo_inline OOObjects( const OOObjects &val ) { *this = val; }
	
	oo_inline OOObjects &operator = ( NSValue *val ) { this->cset( val ); return *this; }
	oo_inline OOObjects &operator = ( const OOObjects &val ) { cset( val.get() ); return *this; }
	oo_inline OOObjects &operator = ( NSMutableArray *array ) {
		for ( int i=0 ; i<[array count] ; i++ )
			(*this)[i] = [array objectAtIndex:i];
		return *this;
	}
	
	oo_inline NSMutableArray *array() {
		OOBuffer<OOReference<RTYPE> > &buff = this->cget();
		int count = buff.bounds ? buff.bounds : buff.used;
		OOArray<RTYPE> array( count );
		for ( int i=0 ; i<count ; i++ )
			array += buff[i];
		return &array;
	}
	oo_inline operator NSMutableArray * () { return array(); }
	oo_inline operator OOArray<RTYPE> () { return array(); }
};

/**
 A fast container for an array of strings.
 <pre>
 OOStrings ted;
 ted[99] <<= @"Go on ";
 ted[99] *= 10; 
 </pre>
 */

class OOStrings : public OOVector< OOString > {
public:
	oo_inline OOStrings() {}
	oo_inline OOStrings( NSMutableArray *arr ) { *this = arr; }
	oo_inline OOStrings( const OOStrings &val ) { *this = val; }
	oo_inline OOStrings( const OOStringArray &array ) { *this = array.get(); }
	
	oo_inline OOStrings &operator = ( NSValue *val ) { this->cset( val ); return *this; }
	oo_inline OOStrings &operator = ( const OOStrings &val ) { cset( val.get() ); return *this; }
	oo_inline OOStrings &operator = ( NSMutableArray *array ) {
		for ( int i=0 ; i<[array count] ; i++ )
			(*this)[i] = [array objectAtIndex:i];
		return *this;
	}
	
	oo_inline NSMutableArray *array() {
		OOBuffer<OOString> &buff = this->cget();
		int count = buff.bounds ? buff.bounds : buff.used;
		OOArray<OOString> array( count );
		for ( int i=0 ; i<count ; i++ )
			array += buff[i];
		return &array;
	}
	oo_inline operator OOStringArray () { return array(); }
};

/*===============================================================================*/
/*============================== Class containers ===============================*/

/**
 A wrapper for an array of heap allocated C++ class pointers stored as NSValue objects.
 
 Usage:
 <pre>
 \@interface ExampleClass {
 OOClassArray<aclass> CPPinstances;
 }
 
 \@implementation 
 
 - function {
 for ( int i=0 ; i<100 ; i+=10 )
 CPPinstances[i] = new aclass();
 ~CPPinstances; /// instances are deleted
 }
 
 \@end
 
 - function {
 OOClassArray<aclass> x;
 x[99] = new aclass();
 } // instance deleteed 
 
 </pre>
 */

template <typename CNAME>
class OOClassArray : public OOArray<OOClassPtr<CNAME> > {
protected:
	// delete each element to call its destructor
	oo_inline NSMutableArray *set( NSMutableArray *val ) {
		NSMutableArray *old = !*this ? nil : this->get();
		if ( old && val != old && [old retainCount] == 1 )
			while ( [this->get() count] )
				~(*this)[-1];
		return OOArray<OOClassPtr<CNAME> >::set( val );
	}
	oo_inline void detruct() { set( nil ); }
public:
	oo_inline OOClassArray() {}
	oo_inline OOClassArray( NSMutableArray *val ) { set( val ); }
	oo_inline OOClassArray( const OOClassArray &val ) { set( val.get() ); }
	oo_inline ~OOClassArray() { this->detruct(); }
	
	oo_inline OOClassArray &operator = ( NSMutableArray *val ) { set( val ); return *this; };
	oo_inline OOClassArray &operator = ( OOClassArray *val ) { set( val.get() ); return *this; };
	oo_inline OOClassArray<CNAME> operator ~ () {
		OOClassArray<CNAME> save = this->get();
		detruct();
		return save;
	}
};

/**
 NSMutable dictionary wrapper for storing heap allocated C++ instances by key.
 
 Usage:
 <pre>
 OOClassDict<aclass> a
 a[@"KEY"] = new aclass[];
 ~a[@"KEY"] // instance deleted
 </pre>
 */

template <typename CNAME>
class OOClassDict : public OODictionary<OOClassPtr<CNAME> > {
protected:
	// delete each entry to call its detructor
	oo_inline NSMutableDictionary *set( NSMutableDictionary *val ) {
		NSMutableDictionary *old = !*this ? nil : this->get();
		if ( old && val != old && [old retainCount] == 1 ) {
#if 0000
			for ( id key in [old allKeys] )
				~(*this)[key];
#else
			OOArray<id> keys = [old allKeys];
			for ( int i=0 ; i<keys ; i++ )
				~(*this)[*keys[i]];
#endif
		}
		return OODictionary<OOClassPtr<CNAME> >::set( val );
	}
	oo_inline void detruct() { set( nil ); }
public:
	oo_inline OOClassDict() {}
	oo_inline OOClassDict( NSMutableDictionary *val ) { set( val ); }
	oo_inline OOClassDict( const OOClassDict &val ) { set( val.get() ); }
	oo_inline ~OOClassDict() { this->detruct(); }

	oo_inline OOClassDict &operator = ( NSMutableArray *val ) { this->set( val ); return *this; };
	oo_inline OOClassDict &operator = ( const OOClassDict &val ) { this->set( val.get() ); return *this; };
	oo_inline OOClassDict<CNAME> operator ~ () {
		OOClassDict<CNAME> save = this->get();
		detruct();
		return save;
	}
};
