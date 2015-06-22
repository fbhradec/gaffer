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

#include "boost/algorithm/string/predicate.hpp"

#include "Gaffer/StringPlug.h"

#include "GafferScene/SubTree.h"
#include "GafferScene/PathMatcherData.h"

using namespace std;
using namespace IECore;
using namespace Gaffer;
using namespace GafferScene;

IE_CORE_DEFINERUNTIMETYPED( SubTree );

size_t SubTree::g_firstPlugIndex = 0;

SubTree::SubTree( const std::string &name )
	:	SceneProcessor( name )
{
	storeIndexOfNextChild( g_firstPlugIndex );
	addChild( new StringPlug( "root", Plug::In, "" ) );
	addChild( new BoolPlug( "includeRoot", Plug::In, false ) );

	// Fast pass-throughs for things we don't modify.
	outPlug()->globalsPlug()->setInput( inPlug()->globalsPlug() );
	outPlug()->setNamesPlug()->setInput( inPlug()->setNamesPlug() );
}

SubTree::~SubTree()
{
}

Gaffer::StringPlug *SubTree::rootPlug()
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

const Gaffer::StringPlug *SubTree::rootPlug() const
{
	return getChild<StringPlug>( g_firstPlugIndex );
}

Gaffer::BoolPlug *SubTree::includeRootPlug()
{
	return getChild<BoolPlug>( g_firstPlugIndex + 1 );
}

const Gaffer::BoolPlug *SubTree::includeRootPlug() const
{
	return getChild<BoolPlug>( g_firstPlugIndex + 1 );
}

void SubTree::affects( const Plug *input, AffectedPlugsContainer &outputs ) const
{
	SceneProcessor::affects( input, outputs );

	if( input->parent<ScenePlug>() == inPlug() )
	{
		outputs.push_back( outPlug()->getChild<ValuePlug>( input->getName() ) );
	}
	else if( input == rootPlug() || input == includeRootPlug() )
	{
		outputs.push_back( outPlug()->boundPlug() );
		outputs.push_back( outPlug()->transformPlug() );
		outputs.push_back( outPlug()->attributesPlug() );
		outputs.push_back( outPlug()->objectPlug() );
		outputs.push_back( outPlug()->childNamesPlug() );
		outputs.push_back( outPlug()->setPlug() );
	}

}

void SubTree::hashBound( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent, IECore::MurmurHash &h ) const
{
	bool createRoot = false;
	ScenePath source = sourcePath( path, createRoot );
	if( createRoot )
	{
		h = hashOfTransformedChildBounds( path, parent );
	}
	else
	{
		h = inPlug()->boundHash( source );
	}
}

Imath::Box3f SubTree::computeBound( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent ) const
{
	bool createRoot = false;
	const ScenePath source = sourcePath( path, createRoot );
	if( createRoot )
	{
		return unionOfTransformedChildBounds( path, parent );
	}
	else
	{
		return inPlug()->bound( source );
	}
}

void SubTree::hashTransform( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent, IECore::MurmurHash &h ) const
{
	bool createRoot = false;
	ScenePath source = sourcePath( path, createRoot );
	assert( !createRoot ); // SceneNode::hash() shouldn't call this for the root path
	h = inPlug()->transformHash( source );
}

Imath::M44f SubTree::computeTransform( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent ) const
{
	bool createRoot = false;
	const ScenePath source = sourcePath( path, createRoot );
	assert( !createRoot ); // SceneNode::compute() shouldn't call this for the root path
	return inPlug()->transform( source );
}

void SubTree::hashAttributes( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent, IECore::MurmurHash &h ) const
{
	bool createRoot = false;
	ScenePath source = sourcePath( path, createRoot );
	assert( !createRoot ); // SceneNode::hash() shouldn't call this for the root path
	h = inPlug()->attributesHash( source );
}

IECore::ConstCompoundObjectPtr SubTree::computeAttributes( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent ) const
{
	bool createRoot = false;
	const ScenePath source = sourcePath( path, createRoot );
	assert( !createRoot ); // SceneNode::compute() shouldn't call this for the root path
	return inPlug()->attributes( source );
}

void SubTree::hashObject( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent, IECore::MurmurHash &h ) const
{
	bool createRoot = false;
	ScenePath source = sourcePath( path, createRoot );
	assert( !createRoot ); // SceneNode::hash() shouldn't call this for the root path
	h = inPlug()->objectHash( source );
}

IECore::ConstObjectPtr SubTree::computeObject( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent ) const
{
	bool createRoot = false;
	const ScenePath source = sourcePath( path, createRoot );
	assert( !createRoot ); // SceneNode::compute() shouldn't call this for the root path
	return inPlug()->object( source );
}

void SubTree::hashChildNames( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent, IECore::MurmurHash &h ) const
{
	bool createRoot = false;
	const ScenePath source = sourcePath( path, createRoot );
	if( createRoot )
	{
		SceneProcessor::hashChildNames( path, context, parent, h );
		h.append( *(source.rbegin()) );
	}
	else
	{
		h = inPlug()->childNamesHash( source );
	}
}

IECore::ConstInternedStringVectorDataPtr SubTree::computeChildNames( const ScenePath &path, const Gaffer::Context *context, const ScenePlug *parent ) const
{
	bool createRoot = false;
	const ScenePath source = sourcePath( path, createRoot );
	if( createRoot )
	{
		IECore::InternedStringVectorDataPtr result = new IECore::InternedStringVectorData;
		result->writable().push_back( *(source.rbegin()) );
		return result;
	}
	else
	{
		return inPlug()->childNames( source );
	}
}

void SubTree::hashSet( const IECore::InternedString &setName, const Gaffer::Context *context, const ScenePlug *parent, IECore::MurmurHash &h ) const
{
	SceneProcessor::hashSet( setName, context, parent, h );
	inPlug()->setPlug()->hash( h );
	rootPlug()->hash( h );
	includeRootPlug()->hash( h );
}

GafferScene::ConstPathMatcherDataPtr SubTree::computeSet( const IECore::InternedString &setName, const Gaffer::Context *context, const ScenePlug *parent ) const
{
	ConstPathMatcherDataPtr inputSetData = inPlug()->setPlug()->getValue();
	const PathMatcher &inputSet = inputSetData->readable();
	if( inputSet.isEmpty() )
	{
		return inputSetData;
	}

	const std::string rootString = rootPlug()->getValue();
	ScenePlug::ScenePath root;
	ScenePlug::stringToPath( rootString, root );

	size_t prefixSize = root.size(); // number of names to remove from front of each path
	if( includeRootPlug()->getValue() && prefixSize )
	{
		prefixSize--;
	}

	/// \todo This could be more efficient if PathMatcher exposed the internal nodes,
	/// and allowed sharing between matchers. Then we could just pick the subtree within
	/// the matcher that we wanted.

	PathMatcherDataPtr outputSetData = new PathMatcherData;
	PathMatcher &outputSet = outputSetData->writable();

	ScenePlug::ScenePath outputPath;
	for( PathMatcher::Iterator pIt = inputSet.begin(), peIt = inputSet.end(); pIt != peIt; ++pIt )
	{
		const ScenePlug::ScenePath &inputPath = *pIt;
		if( boost::starts_with( inputPath, root ) )
		{
			outputPath.assign( inputPath.begin() + prefixSize, inputPath.end() );
			outputSet.addPath( outputPath );
		}
	}

	return outputSetData;
}

SceneNode::ScenePath SubTree::sourcePath( const ScenePath &outputPath, bool &createRoot ) const
{
	/// \todo We should introduce a plug type which stores its values as a ScenePath directly.
	string rootAsString = rootPlug()->getValue();
	ScenePath result;
	ScenePlug::stringToPath( rootAsString, result );

	createRoot = false;
	if( result.size() && includeRootPlug()->getValue() )
	{
		if( outputPath.size() )
		{
			result.insert( result.end(), outputPath.begin() + 1, outputPath.end() );
		}
		else
		{
			createRoot = true;
		}
	}
	else
	{
		result.insert( result.end(), outputPath.begin(), outputPath.end() );
	}

	return result;
}
