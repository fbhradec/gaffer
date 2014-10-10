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

#ifndef GAFFERSCENE_SCENEPROCEDURAL_H
#define GAFFERSCENE_SCENEPROCEDURAL_H

#include "tbb/atomic.h"

#include "IECore/Renderer.h"
#include "IECore/Camera.h"
#include "IECore/Light.h"
#include "IECore/CoordinateSystem.h"

#include "GafferScene/ScenePlug.h"
#include "GafferScene/PathMatcherData.h"

namespace Gaffer
{

IE_CORE_FORWARDDECLARE( Context )
IE_CORE_FORWARDDECLARE( ScriptNode )

} // namespace Gaffer

namespace GafferScene
{

/// The SceneProcedural class passes the output from a ScenePlug to an IECore::Renderer
/// in a tree of nested deferred procedurals. See the python ScriptProcedural for
/// a procedural which will load a gaffer script and generate geometry from a named
/// node.
/// \todo This class is currently being abused by the SceneView for drawing the scene in
/// the viewer, and has some functionality that would otherwise be meaningless. We should
/// instead implement custom scene traversal in the SceneView and simplify the SceneProcedural
/// so it has just enough functionality for final rendering and nothing more. Things we should
/// remove include :
///
///   - the PathMatcher
///   - the minimumExpansionDepth
///   - the drawing of cameras, lights and coordinate systems
///
/// \todo There is useful functionality in here for calculating bounds and outputting things
/// to Renderers that should probably be moved to RendererAlgo.h and/or SceneAlgo.h.
class SceneProcedural : public IECore::Renderer::Procedural
{

	public :

		IE_CORE_DECLAREMEMBERPTR( SceneProcedural );

		/// A copy of context is taken.
		SceneProcedural( ConstScenePlugPtr scenePlug, const Gaffer::Context *context, const ScenePlug::ScenePath &scenePath=ScenePlug::ScenePath(), const PathMatcherData *pathsToExpand=0, size_t minimumExpansionDepth=0 );
		virtual ~SceneProcedural();

		virtual IECore::MurmurHash hash() const;
		virtual Imath::Box3f bound() const;
		virtual void render( IECore::Renderer *renderer ) const;
		
		typedef boost::signal<void ( void )> AllRenderedSignal;
		
		/// A signal emitted when all pending SceneProcedurals have been rendered or destroyed
		static AllRenderedSignal &allRenderedSignal();

	protected :

		SceneProcedural( const SceneProcedural &other, const ScenePlug::ScenePath &scenePath );

		// This class must hold a reference to the script node, to prevent it from being
		// destroyed mid-render.
		Gaffer::ConstScriptNodePtr m_scriptNode;
		ConstScenePlugPtr m_scenePlug;
		Gaffer::ContextPtr m_context;
		ScenePlug::ScenePath m_scenePath;

		PathMatcherDataPtr m_pathsToExpand;
		size_t m_minimumExpansionDepth;

		struct Options
		{
			bool transformBlur;
			bool deformationBlur;
			Imath::V2f shutter;
		};

		Options m_options;

		struct Attributes
		{
			bool transformBlur;
			unsigned transformBlurSegments;
			bool deformationBlur;
			unsigned deformationBlurSegments;
		};

		Attributes m_attributes;

	private :

		void updateAttributes( bool full );
		void motionTimes( unsigned segments, std::set<float> &times ) const;

		void drawCamera( const IECore::Camera *camera, IECore::Renderer *renderer ) const;
		void drawLight( const IECore::Light *light, IECore::Renderer *renderer ) const;
		void drawCoordinateSystem( const IECore::CoordinateSystem *coordinateSystem, IECore::Renderer *renderer ) const;
		
		// A global counter of all the scene procedurals that are hanging around but haven't been rendered yet, which 
		// gets incremented in the constructor and decremented in doRender() or the destructor, whichever happens first.
		// When this counter falls to zero, a signal is emitted, so you can eg clear the cache when procedural expansion
		// has finished during a render.
		static tbb::atomic<int> g_pendingSceneProcedurals;
		
		// Indicates if SceneProcedural::doRender() has been called. If not, g_pendingSceneProcedurals is decremented in the
		// destructor
		mutable bool m_rendered;
		
		void decrementPendingProcedurals() const;
		
		static AllRenderedSignal g_allRenderedSignal;
		
};

IE_CORE_DECLAREPTR( SceneProcedural );

} // namespace GafferScene

#endif // GAFFERSCENE_SCENEPROCEDURAL_H
