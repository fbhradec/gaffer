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

import Gaffer
import GafferUI
import GafferScene

Gaffer.Metadata.registerNode(

	GafferScene.ShaderSwitch,

	"description",
	"""
	Chooses between multiple input shaders, passing through the
	chosen shader to the output. The switching is resolved
	before rendering begins, so no per-sample overhead is
	incurred during shading.
	""",

	"nodeGadget:minWidth", 0.0,

	plugs = {

		"in*" : [

			"nodeGadget:nodulePosition", "left",

		],

		"in" : [

			"description",
			"""
			The first input shader - the one passed through when
			the index is 0.
			""",

		],

		"out" : [

			"description",
			"""
			The output shader.
			""",

			"nodeGadget:nodulePosition", "right",

		],

		"index" : [

			"nodule:type", "",

		],

	},

)

GafferUI.PlugValueWidget.registerCreator( GafferScene.ShaderSwitch, "in", None )
GafferUI.PlugValueWidget.registerCreator( GafferScene.ShaderSwitch, "in[0-9]*", None )
GafferUI.PlugValueWidget.registerCreator( GafferScene.ShaderSwitch, "out", None )
