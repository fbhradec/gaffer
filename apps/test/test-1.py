##########################################################################
#  
#  Copyright (c) 2011-2012, John Haddon. All rights reserved.
#  Copyright (c) 2011-2013, Image Engine Design Inc. All rights reserved.
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

class test( Gaffer.Application ) :

	def __init__( self ) :
	
		Gaffer.Application.__init__( self )
		
		self.parameters().addParameters(
		
			[
				IECore.StringVectorParameter(
					name = "testCases",
					description = "A list of names of specific test cases to run. If unspecified then all test cases are run.",
					defaultValue = IECore.StringVectorData(),
				),
				
				IECore.IntParameter(
					name = "repeat",
					description = "The number of times to repeat the tests.",
					defaultValue = 1,
				),
			]
		
		)
		
		self.parameters().userData()["parser"] = IECore.CompoundObject(
			{
				"flagless" : IECore.StringVectorData( [ "testCases" ] )
			}
		)
				
	def _run( self, args ) :
		
		import unittest

		testSuite = unittest.TestSuite()
		if args["testCases"] :
		
			for name in args["testCases"] :
				testCase = unittest.defaultTestLoader.loadTestsFromName( name )
				testSuite.addTest( testCase )
			
		else :
		
			import GafferTest
			import GafferUITest
			import GafferSceneTest
			import GafferImageTest
			import GafferImageUITest
		
			for module in ( GafferTest, GafferUITest, GafferSceneTest, GafferImageTest, GafferImageUITest ) :
		
				moduleTestSuite = unittest.defaultTestLoader.loadTestsFromModule( module )
				testSuite.addTest( moduleTestSuite )
			
		for i in range( 0, args["repeat"].value ) :
			testRunner = unittest.TextTestRunner( verbosity=2 )
			testResult = testRunner.run( testSuite )
			if not testResult.wasSuccessful() :
				return 1
			
		return 0

IECore.registerRunTimeTyped( test )
