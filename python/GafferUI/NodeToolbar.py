##########################################################################
#  
#  Copyright (c) 2014, Image Engine Design Inc. All rights reserved.
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

## Abstract base class for toolbars representing nodes - 
# see StandardNodeToolbar for a concrete implementation
# suitable for most purposes.
class NodeToolbar( GafferUI.Widget ) :
	
	def __init__( self, node, topLevelWidget, **kw ) :

		GafferUI.Widget.__init__( self, topLevelWidget, **kw )
	
		self.__node = node
			
	## Returns the node the toolbar represents.
	def node( self ) :
	
		return self.__node

	## Creates a NodeToolbar instance for the specified node.
	# Note that not all nodes have toolbars, so None may be returned.
	@classmethod
	def create( cls, node ) :
	
		nodeHierarchy = IECore.RunTimeTyped.baseTypeIds( node.typeId() )
		for typeId in [ node.typeId() ] + nodeHierarchy :	
			creator = cls.__creators.get( typeId, None )
			if creator is not None :
				return creator( node )
		
		return None
	
	__creators = {}
	## Registers a subclass of NodeToolbar to be used with a specific node type.
	@classmethod
	def registerCreator( cls, nodeTypeId, toolbarCreator ) :
	
		assert( callable( toolbarCreator ) )
	
		cls.__creators[nodeTypeId] = toolbarCreator
