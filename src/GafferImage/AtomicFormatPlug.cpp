//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012-2015, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      * Redistributions of source code must retain the above
//        copyright notice, this list of conditions and the following
//        disclaimer.
//
//      * Redistributions in binary form must reproduce the above
//        copyright notice, this list of conditions and the following
//        disclaimer in the documentation and/or other materials provided with
//        the distribution.
//
//      * Neither the name of John Haddon nor the names of
//        any other contributors to this software may be used to endorse or
//        promote products derived from this software without specific prior
//        written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "Gaffer/Context.h"
#include "Gaffer/TypedPlug.h"
#include "Gaffer/TypedPlug.inl"
#include "Gaffer/Process.h"

#include "GafferImage/FormatData.h"
#include "GafferImage/AtomicFormatPlug.h"
#include "GafferImage/FormatPlug.h"

using namespace Gaffer;
using namespace GafferImage;

namespace Gaffer
{

IECORE_RUNTIMETYPED_DEFINETEMPLATESPECIALISATION( GafferImage::AtomicFormatPlug, AtomicFormatPlugTypeId )

template<>
Format AtomicFormatPlug::getValue( const IECore::MurmurHash *precomputedHash ) const
{
	IECore::ConstObjectPtr o = getObjectValue( precomputedHash );
	const GafferImage::FormatData *d = IECore::runTimeCast<const GafferImage::FormatData>( o.get() );
	if( !d )
	{
		throw IECore::Exception( "AtomicFormatPlug::getObjectValue() didn't return FormatData - is the hash being computed correctly?" );
	}

	Format result = d->readable();
	if( result.getDisplayWindow().isEmpty() && ( ( direction() == Plug::In && Process::current() ) || direction() == Plug::Out ) )
	{
		return FormatPlug::getDefaultFormat( Context::current() );
	}
	return result;
}

template<>
IECore::MurmurHash AtomicFormatPlug::hash() const
{
	const AtomicFormatPlug *p = source<AtomicFormatPlug>();

	if( p->direction() == Plug::In )
	{
		const Format v = p->getValue();

		IECore::MurmurHash result;
		result.append( v.getDisplayWindow() );
		result.append( v.getPixelAspect() );
		return result;
	}

	return p->ValuePlug::hash();
}

template class TypedPlug<GafferImage::Format>;

} // namespace Gaffer