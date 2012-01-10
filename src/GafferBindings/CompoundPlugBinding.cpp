//////////////////////////////////////////////////////////////////////////
//  
//  Copyright (c) 2011-2012, John Haddon. All rights reserved.
//  Copyright (c) 2011, Image Engine Design Inc. All rights reserved.
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

#include "boost/python.hpp"

#include "GafferBindings/CompoundPlugBinding.h"
#include "GafferBindings/Serialiser.h"
#include "GafferBindings/PlugBinding.h"
#include "Gaffer/CompoundPlug.h"

#include "IECorePython/RunTimeTypedBinding.h"
#include "IECorePython/Wrapper.h"

using namespace boost::python;
using namespace GafferBindings;
using namespace Gaffer;

class CompoundPlugWrapper : public CompoundPlug, public IECorePython::Wrapper<CompoundPlug>
{

	public :
	
		CompoundPlugWrapper( PyObject *self, const std::string &name, Direction direction, unsigned flags, tuple children )
			:	CompoundPlug( name, direction, flags ), IECorePython::Wrapper<CompoundPlug>( self, this )
		{
			size_t s = extract<size_t>( children.attr( "__len__" )() );
			for( size_t i=0; i<s; i++ )
			{
				PlugPtr c = extract<PlugPtr>( children[i] );
				addChild( c );
			}
		}

		GAFFERBINDINGS_PLUGWRAPPERFNS( CompoundPlug )

};

IE_CORE_DECLAREPTR( CompoundPlugWrapper );

static std::string serialise( Serialiser &s, ConstGraphComponentPtr g )
{
	ConstCompoundPlugPtr plug = IECore::staticPointerCast<const CompoundPlug>( g );
	std::string result = s.modulePath( g ) + "." + g->typeName() + "( \"" + g->getName() + "\", ";
	
	if( plug->direction()!=Plug::In )
	{
		result += "direction = " + serialisePlugDirection( plug->direction() ) + ", ";
	}
		
	if( plug->getFlags() != Plug::Default )
	{
		result += "flags = " + serialisePlugFlags( plug->getFlags() ) + ", ";
	}
	
	if( plug->children().size() )
	{
		result += "children = ( ";
	
		PlugIterator pIt( plug->children().begin(), plug->children().end() );
		while( pIt!=plug->children().end() )
		{
			result += s.serialiseC( *pIt++ ) + ", ";
		}
	
		result += " )";
	}
		
	result += " )";

	return result;
}

void GafferBindings::bindCompoundPlug()
{
	IECorePython::RunTimeTypedClass<CompoundPlug, CompoundPlugWrapperPtr>()
		.def(	init< const std::string &, Plug::Direction, unsigned, tuple >
				(
					(
						arg( "name" ) = CompoundPlug::staticTypeName(),
						arg( "direction" ) = Plug::In,
						arg( "flags" ) = Plug::Default,
						arg( "children" )=tuple()
					)
				)	
		)
		.GAFFERBINDINGS_DEFPLUGWRAPPERFNS( CompoundPlug )
	;
	
	Serialiser::registerSerialiser( CompoundPlug::staticTypeId(), serialise );
}
