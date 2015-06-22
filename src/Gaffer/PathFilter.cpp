//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, John Haddon. All rights reserved.
//  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

#include "Gaffer/Path.h"
#include "Gaffer/PathFilter.h"

using namespace std;
using namespace IECore;
using namespace Gaffer;

IE_CORE_DEFINERUNTIMETYPED( PathFilter );

PathFilter::PathFilter( IECore::CompoundDataPtr userData )
	:	m_userData( userData ), m_enabled( true )
{
}

PathFilter::~PathFilter()
{
}

IECore::CompoundData *PathFilter::userData()
{
	if( !m_userData )
	{
		m_userData = new CompoundData;
	}
	return m_userData.get();
}

void PathFilter::setEnabled( bool enabled )
{
	if( enabled == m_enabled )
	{
		return;
	}

	m_enabled = enabled;
	m_changedSignal( this );
}

bool PathFilter::getEnabled() const
{
	return m_enabled;
}

void PathFilter::filter( std::vector<PathPtr> &paths ) const
{
	if( !m_enabled )
	{
		return;
	}
	doFilter( paths );
}

typedef boost::signal<void ( PathFilter * )> ChangedSignal;
ChangedSignal &PathFilter::changedSignal()
{
	return m_changedSignal;
}

void PathFilter::doFilter( std::vector<PathPtr> &paths ) const
{
}
