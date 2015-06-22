//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2012, John Haddon. All rights reserved.
//  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
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

#include "boost/tokenizer.hpp"

#include "IECorePython/RunTimeTypedBinding.h"

#include "GafferBindings/PlugBinding.h"

#include "GafferScene/ScenePlug.h"

#include "GafferSceneBindings/ScenePlugBinding.h"

using namespace boost::python;
using namespace Gaffer;
using namespace GafferBindings;
using namespace GafferScene;
using namespace GafferSceneBindings;

namespace
{

// ScenePlug::ScenePath is just a typedef for std::vector<InternedString>,
// which doesn't exist in Python. So we register a conversion from
// InternedStringVectorData which contains just such a vector.
/// \todo We could instead do this in the Cortex bindings for all
/// VectorTypedData types.
struct ScenePathFromInternedStringVectorData
{

	ScenePathFromInternedStringVectorData()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			NULL,
			boost::python::type_id<ScenePlug::ScenePath>()
		);
	}

	static void *convertible( PyObject *obj )
	{
		extract<IECore::InternedStringVectorData *> dataExtractor( obj );
		if( dataExtractor.check() )
		{
			if( IECore::InternedStringVectorData *data = dataExtractor() )
			{
				return &(data->writable());
			}
		}

		return NULL;
	}

};

// As a convenience we also accept strings in place of ScenePaths when
// calling from python. We deliberately don't do the same in c++ to force
// people to use the faster form.
struct ScenePathFromString
{

	ScenePathFromString()
	{
		boost::python::converter::registry::push_back(
			&convertible,
			&construct,
			boost::python::type_id<ScenePlug::ScenePath>()
		);
	}

	static void *convertible( PyObject *obj )
	{
		if( PyString_Check( obj ) )
		{
			return obj;
		}
		return NULL;
	}

	static void construct( PyObject *obj, boost::python::converter::rvalue_from_python_stage1_data *data )
	{
		void *storage = (( converter::rvalue_from_python_storage<ScenePlug::ScenePath>* ) data )->storage.bytes;
		ScenePlug::ScenePath *path = new( storage ) ScenePlug::ScenePath();
		data->convertible = storage;

		std::string s = extract<std::string>( obj );
		typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
		Tokenizer t( s, boost::char_separator<char>( "/" ) );
		for( Tokenizer::const_iterator it = t.begin(), eIt = t.end(); it != eIt; it++ )
		{
			path->push_back( *it );
		}
	}

};

Imath::Box3f boundWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.bound( scenePath );
}

Imath::M44f transformWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.transform( scenePath );
}

Imath::M44f fullTransformWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.fullTransform( scenePath );
}

IECore::ObjectPtr objectWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath, bool copy=true )
{
	IECorePython::ScopedGILRelease gilRelease;
	IECore::ConstObjectPtr o = plug.object( scenePath );
	return copy ? o->copy() : boost::const_pointer_cast<IECore::Object>( o );
}

IECore::InternedStringVectorDataPtr childNamesWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath, bool copy=true )
{
	IECorePython::ScopedGILRelease gilRelease;
	IECore::ConstInternedStringVectorDataPtr n = plug.childNames( scenePath );
	return copy ? n->copy() : boost::const_pointer_cast<IECore::InternedStringVectorData>( n );
}

IECore::CompoundObjectPtr attributesWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath, bool copy=true )
{
	IECorePython::ScopedGILRelease gilRelease;
	IECore::ConstCompoundObjectPtr a = plug.attributes( scenePath );
	return copy ? a->copy() : boost::const_pointer_cast<IECore::CompoundObject>( a );
}

IECore::CompoundObjectPtr fullAttributesWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.fullAttributes( scenePath );
}

PathMatcherDataPtr setWrapper( const ScenePlug &plug, const IECore::InternedString &setName, bool copy=true )
{
	IECorePython::ScopedGILRelease gilRelease;
	ConstPathMatcherDataPtr s = plug.set( setName );
	return copy ? s->copy() : boost::const_pointer_cast<PathMatcherData>( s );
}

IECore::MurmurHash boundHashWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.boundHash( scenePath );
}

IECore::MurmurHash transformHashWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.transformHash( scenePath );
}

IECore::MurmurHash fullTransformHashWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.fullTransformHash( scenePath );
}

IECore::MurmurHash objectHashWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.objectHash( scenePath );
}

IECore::MurmurHash childNamesHashWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.childNamesHash( scenePath );
}

IECore::MurmurHash attributesHashWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.attributesHash( scenePath );
}

IECore::MurmurHash fullAttributesHashWrapper( const ScenePlug &plug, const ScenePlug::ScenePath &scenePath )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.fullAttributesHash( scenePath );
}

IECore::MurmurHash setHashWrapper( const ScenePlug &plug, const IECore::InternedString &setName )
{
	IECorePython::ScopedGILRelease gilRelease;
	return plug.setHash( setName );
}

IECore::InternedStringVectorDataPtr stringToPathWrapper( const char *s )
{
	IECore::InternedStringVectorDataPtr p = new IECore::InternedStringVectorData;
	ScenePlug::stringToPath( s, p->writable() );
	return p;
}

} // namespace

void GafferSceneBindings::bindScenePlug()
{

	PlugClass<ScenePlug>()
		.def( init<const std::string &, Plug::Direction, unsigned>(
				(
					arg( "name" ) = Gaffer::GraphComponent::defaultName<ScenePlug>(),
					arg( "direction" ) = Gaffer::Plug::In,
					arg( "flags" ) = Gaffer::Plug::Default
				)
			)
		)
		// value accessors
		.def( "bound", &boundWrapper )
		.def( "transform", &transformWrapper )
		.def( "fullTransform", &fullTransformWrapper )
		.def( "object", &objectWrapper, ( boost::python::arg_( "_copy" ) = true ) )
		.def( "childNames", &childNamesWrapper, ( boost::python::arg_( "_copy" ) = true ) )
		.def( "attributes", &attributesWrapper, ( boost::python::arg_( "_copy" ) = true ) )
		.def( "fullAttributes", &fullAttributesWrapper )
		.def( "set", &setWrapper, ( boost::python::arg_( "_copy" ) = true ) )
		// hash accessors
		.def( "boundHash", &boundHashWrapper )
		.def( "transformHash", &transformHashWrapper )
		.def( "fullTransformHash", &fullTransformHashWrapper )
		.def( "objectHash", &objectHashWrapper )
		.def( "childNamesHash", &childNamesHashWrapper )
		.def( "attributesHash", &attributesHashWrapper )
		.def( "fullAttributesHash", &fullAttributesHashWrapper )
		.def( "setHash", &setHashWrapper )
		// string utilities
		.def( "stringToPath", &stringToPathWrapper )
		.staticmethod( "stringToPath" )
		.def( "pathToString", &ScenePlug::pathToString )
		.staticmethod( "pathToString" )
;

	ScenePathFromInternedStringVectorData();
	ScenePathFromString();

}
