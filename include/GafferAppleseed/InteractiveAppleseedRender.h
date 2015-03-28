//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2015, Esteban Tovagliari. All rights reserved.
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

#ifndef GAFFERSCENE_INTERACTIVEAPPLESEEDRENDER_H
#define GAFFERSCENE_INTERACTIVEAPPLESEEDRENDER_H

#include "GafferScene/InteractiveRender.h"

#include "GafferAppleseed/TypeIds.h"

namespace GafferAppleseed
{

class InteractiveAppleseedRender : public GafferScene::InteractiveRender
{

	public :

		InteractiveAppleseedRender( const std::string &name=defaultName<InteractiveAppleseedRender>() );
		virtual ~InteractiveAppleseedRender();

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( GafferAppleseed::InteractiveAppleseedRender, InteractiveAppleseedRenderTypeId, GafferScene::InteractiveRender );

	protected :

		/// Must be implemented by derived classes to return the renderer that will be used.
		virtual IECore::RendererPtr createRenderer() const;

};

IE_CORE_DECLAREPTR( InteractiveAppleseedRender );

} // namespace GafferAppleseed

#endif // GAFFERSCENE_INTERACTIVEAPPLESEEDRENDER_H