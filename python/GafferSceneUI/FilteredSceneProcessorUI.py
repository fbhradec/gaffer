##########################################################################
#
#  Copyright (c) 2013, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#      * Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      * Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials provided with
#        the distribution.
#
#      * Neither the name of John Haddon nor the names of
#        any other contributors to this software may be used to endorse or
#        promote products derived from this software without specific prior
#        written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import IECore

import Gaffer
import GafferUI

import GafferScene
import GafferSceneUI

##########################################################################
# Metadata
##########################################################################

Gaffer.Metadata.registerNode(

	GafferScene.FilteredSceneProcessor,

	"description",
	"""
	The base type for scene processors which use a Filter node to control
	which part of the scene is affected.
	""",

	plugs = {

		"filter" : [

			"description",
			"""
			The filter used to control which parts of the scene are
			processed. A Filter node should be connected here.
			""",

			"layout:section", "Filter",
			"nodeGadget:nodulePosition", "right",
			"layout:index", -3, # Just before the enabled plug,
			"nodule:type", "GafferUI::StandardNodule",

		],

	},

)

##########################################################################
# Widgets and Gadgets
##########################################################################

GafferUI.PlugValueWidget.registerCreator(
	GafferScene.FilteredSceneProcessor,
	"filter",
	GafferSceneUI.FilterPlugValueWidget,
)

def __nodeGadget( node ) :

	nodeGadget = GafferUI.StandardNodeGadget( node )
	GafferSceneUI.PathFilterUI.addObjectDropTarget( nodeGadget )

	return nodeGadget

GafferUI.NodeGadget.registerNodeGadget( GafferScene.FilteredSceneProcessor, __nodeGadget )

##########################################################################
# NodeGraph context menu
##########################################################################

def __selectAffected( node, context ) :

	if isinstance( node, GafferScene.FilteredSceneProcessor ) :
		filter = node["filter"]
		scenes = [ node["in"] ]
	else :
		filter = node
		scenes = []
		def walkOutputs( plug ) :
			for output in plug.outputs() :
				node = output.node()
				if isinstance( node, GafferScene.FilteredSceneProcessor ) and output.isSame( node["filter"] ) :
					scenes.append( node["in"] )
				walkOutputs( output )

		walkOutputs( filter["out"] )

	pathMatcher = GafferScene.PathMatcher()
	with context :
		for scene in scenes :
			GafferScene.matchingPaths( filter, scene, pathMatcher )

	context["ui:scene:selectedPaths"] = IECore.StringVectorData( pathMatcher.paths() )

def appendNodeContextMenuDefinitions( nodeGraph, node, menuDefinition ) :

	if not isinstance( node, ( GafferScene.FilteredSceneProcessor, GafferScene.Filter ) ) :
		return

	menuDefinition.append( "/FilteredSceneProcessorDivider", { "divider" : True } )
	menuDefinition.append( "/Select Affected Objects", { "command" : IECore.curry( __selectAffected, node, nodeGraph.getContext() ) } )

##########################################################################
# NodeEditor tool menu
##########################################################################

def appendNodeEditorToolMenuDefinitions( nodeEditor, node, menuDefinition ) :

	if not isinstance( node, ( GafferScene.FilteredSceneProcessor, GafferScene.Filter ) ) :
		return

	menuDefinition.append( "/FilteredSceneProcessorDivider", { "divider" : True } )
	menuDefinition.append( "/Select Affected Objects", { "command" : IECore.curry( __selectAffected, node, nodeEditor.getContext() ) } )
