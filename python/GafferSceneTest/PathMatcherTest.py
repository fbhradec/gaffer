##########################################################################
#
#  Copyright (c) 2012, John Haddon. All rights reserved.
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

import unittest
import random

import IECore

import Gaffer
import GafferScene
import GafferSceneTest

class PathMatcherTest( GafferSceneTest.SceneTestCase ) :

	@staticmethod
	def generatePaths( seed, depthRange, numChildrenRange ) :

		nouns = [
			"Ball", "Building", "Car", "Tree", "Rampart", "Head", "Arm",
			"Window", "Door", "Trailer", "Light", "FlockOfBirds", "Herd", "Sheep",
			"Cow", "Wing", "Engine", "Mast", "Rock", "Road", "Sign",
		]

		adjectives = [
			"big", "red", "metallic", "left", "right", "top", "bottom", "wooden",
			"front", "back", "lower", "upper", "magnificent", "hiRes", "loRes",
		]

		paths = []
		def buildWalk( parent=IECore.InternedStringVectorData(), depth=1 ) :

			if depth > random.randint( *depthRange ) :
				return

			for i in range( 0, random.randint( *numChildrenRange ) ) :
				path = parent.copy()
				path.append( random.choice( adjectives ) + random.choice( nouns ) + str( i ) )
				paths.append( path )
				buildWalk( path, depth + 1 )

		random.seed( seed )
		buildWalk()

		return paths

	def testMatch( self ) :

		m = GafferScene.PathMatcher( [ "/a", "/red", "/b/c/d" ] )

		for path, result in [
			( "/a", GafferScene.Filter.Result.ExactMatch ),
			( "/red", GafferScene.Filter.Result.ExactMatch ),
			( "/re", GafferScene.Filter.Result.NoMatch ),
			( "/redThing", GafferScene.Filter.Result.NoMatch ),
			( "/b/c/d", GafferScene.Filter.Result.ExactMatch ),
			( "/c", GafferScene.Filter.Result.NoMatch ),
			( "/a/b", GafferScene.Filter.Result.AncestorMatch ),
			( "/blue", GafferScene.Filter.Result.NoMatch ),
			( "/b/c", GafferScene.Filter.Result.DescendantMatch ),
		] :
			self.assertEqual( m.match( path ), result )

	def testLookupScaling( self ) :

		# this test provides a useful means of measuring performance when
		# working on the PatchMatcher algorithm. it tests matchers
		# for each of two different hierarchies :
		#
		#    * a deep hierarchy with relatively few children at each branch point
		#	 * a shallow hierarchy with large numbers of children at each branch point
		#
		# the tests build a matcher, and then assert that every path in the hierarchy is
		# matched appropriately. uncomment the timers to get useful information printed out.

		match = GafferScene.Filter.Result.ExactMatch

		# deep hierarchy
		paths = self.generatePaths( seed = 10, depthRange = ( 3, 14 ), numChildrenRange = ( 2, 6 ) )
 		t = IECore.Timer()
		matcher = GafferScene.PathMatcher( paths )
 		#print "BUILD DEEP", t.stop()

 		t = IECore.Timer()
		for path in paths :
			self.assertTrue( matcher.match( path ) & match )
		#print "LOOKUP DEEP", t.stop()

		# shallow hierarchy
		paths = self.generatePaths( seed = 10, depthRange = ( 2, 2 ), numChildrenRange = ( 500, 1000 ) )
 		t = IECore.Timer()
		matcher = GafferScene.PathMatcher( paths )
 		#print "BUILD SHALLOW", t.stop()

 		t = IECore.Timer()
		for path in paths :
			self.assertTrue( matcher.match( path ) & match )
		#print "LOOKUP SHALLOW", t.stop()

	def testDefaultConstructor( self ) :

		m = GafferScene.PathMatcher()
		self.assertEqual( m.match( "/" ), GafferScene.Filter.Result.NoMatch )

	def testWildcards( self ) :

		f = GafferScene.PathFilter()
		f["paths"].setValue(
			IECore.StringVectorData( [
				"/a",
				"/red*",
				"/green*Bloke*",
				"/somewhere/over/the/*",
				"/somewhere/over/the/*/skies/are/blue",
			] )
		)

		for path, result in [
			( "/a", f.Result.ExactMatch ),
			( "/redBoots", f.Result.ExactMatch ),
			( "/red", f.Result.ExactMatch ),
			( "/redWellies", f.Result.ExactMatch ),
			( "/redWellies/in/puddles", f.Result.AncestorMatch ),
			( "/greenFatBloke", f.Result.ExactMatch ),
			( "/greenBloke", f.Result.ExactMatch ),
			( "/greenBlokes", f.Result.ExactMatch ),
			( "/somewhere/over/the/rainbow", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/somewhere/over/the", f.Result.DescendantMatch ),
			( "/somewhere/over", f.Result.DescendantMatch ),
			( "/somewhere", f.Result.DescendantMatch ),
			( "/somewhere/over/the/rainbow/skies/are/blue", f.Result.ExactMatch | f.Result.AncestorMatch ),
			( "/somewhere/over/the/rainbow/skies/are", f.Result.DescendantMatch | f.Result.AncestorMatch ),
			( "/somewhere/over/the/astonExpressway/skies/are", f.Result.DescendantMatch | f.Result.AncestorMatch ),
			( "/somewhere/over/the/astonExpressway/skies/are/blue", f.Result.ExactMatch | f.Result.AncestorMatch ),
			( "/somewhere/over/the/astonExpressway/skies/are/grey", f.Result.AncestorMatch ),
		] :

			c = Gaffer.Context()
			c["scene:path"] = IECore.InternedStringVectorData( path[1:].split( "/" ) )
			with c :
				self.assertEqual( f["out"].getValue(), int( result ) )

	def testWildcardsWithSiblings( self ) :

		f = GafferScene.PathFilter()
		f["paths"].setValue(
			IECore.StringVectorData( [
				"/a/*/b",
				"/a/a*/c",
			] )
		)

		for path, result in [
			( "/a/aThing/c", f.Result.ExactMatch ),
			( "/a/aThing/b", f.Result.ExactMatch ),
		] :

			c = Gaffer.Context()
			c["scene:path"] = IECore.InternedStringVectorData( path[1:].split( "/" ) )
			with c :
				self.assertEqual( f["out"].getValue(), int( result ) )

	def testRepeatedWildcards( self ) :

		f = GafferScene.PathFilter()
		f["paths"].setValue(
			IECore.StringVectorData( [
				"/a/**s",
			] )
		)

		c = Gaffer.Context()
		c["scene:path"] = IECore.InternedStringVectorData( [ "a", "s" ] )
		with c :
			self.assertEqual( f["out"].getValue(), int( GafferScene.Filter.Result.ExactMatch ) )

	def testEllipsis( self ) :

		f = GafferScene.PathFilter()
		f["paths"].setValue(
			IECore.StringVectorData( [
				"/a/.../b*",
				"/a/c",
			] )
		)

		for path, result in [
			( "/a/ball", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/red/ball", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/red/car", f.Result.DescendantMatch ),
			( "/a/big/red/ball", f.Result.ExactMatch | f.Result.DescendantMatch | f.Result.AncestorMatch ),
			( "/a/lovely/shiny/bicyle", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/c", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/d", f.Result.DescendantMatch ),
			( "/a/anything", f.Result.DescendantMatch ),
			( "/a/anything/really", f.Result.DescendantMatch ),
			( "/a/anything/at/all", f.Result.DescendantMatch ),
			( "/b/anything/at/all", f.Result.NoMatch ),
		] :

			c = Gaffer.Context()
			c["scene:path"] = IECore.InternedStringVectorData( path[1:].split( "/" ) )
			with c :
				self.assertEqual( f["out"].getValue(), int( result ) )

	def testEllipsisWithMultipleBranches( self ) :

		f = GafferScene.PathFilter()
		f["paths"].setValue(
			IECore.StringVectorData( [
				"/a/.../b*",
				"/a/.../c*",
			] )
		)

		for path, result in [
			( "/a/ball", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/red/ball", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/red/car", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/big/red/ball", f.Result.ExactMatch | f.Result.DescendantMatch | f.Result.AncestorMatch ),
			( "/a/lovely/shiny/bicyle", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/c", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/d", f.Result.DescendantMatch ),
			( "/a/anything", f.Result.DescendantMatch ),
			( "/a/anything/really", f.Result.DescendantMatch ),
			( "/a/anything/at/all", f.Result.DescendantMatch ),
			( "/b/anything/at/all", f.Result.NoMatch ),
		] :

			c = Gaffer.Context()
			c["scene:path"] = IECore.InternedStringVectorData( path[1:].split( "/" ) )
			with c :
				self.assertEqual( f["out"].getValue(), int( result ) )

	def testEllipsisAsTerminator( self ) :

		f = GafferScene.PathFilter()
		f["paths"].setValue(
			IECore.StringVectorData( [
				"/a/...",
			] )
		)

		for path, result in [
			( "/a", f.Result.ExactMatch | f.Result.DescendantMatch ),
			( "/a/ball", f.Result.ExactMatch | f.Result.DescendantMatch | f.Result.AncestorMatch ),
			( "/a/red/car", f.Result.ExactMatch | f.Result.DescendantMatch | f.Result.AncestorMatch ),
			( "/a/red/car/rolls", f.Result.ExactMatch | f.Result.DescendantMatch | f.Result.AncestorMatch ),
			( "/a/terminating/ellipsis/matches/everything/below/it", f.Result.ExactMatch | f.Result.DescendantMatch | f.Result.AncestorMatch ),
		 ] :

			c = Gaffer.Context()
			c["scene:path"] = IECore.InternedStringVectorData( path[1:].split( "/" ) )
			with c :
				self.assertEqual( f["out"].getValue(), int( result ) )

	def testCopyConstructorAppearsDeep( self ) :

		m = GafferScene.PathMatcher( [ "/a" ] )
		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.ExactMatch )

		m2 = GafferScene.PathMatcher( m )
		self.assertEqual( m2.match( "/a" ), GafferScene.Filter.Result.ExactMatch )

		m.clear()
		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.NoMatch )

		self.assertEqual( m2.match( "/a" ), GafferScene.Filter.Result.ExactMatch )

	def testAddAndRemovePaths( self ) :

		m = GafferScene.PathMatcher()
		m.addPath( "/a" )
		m.addPath( "/a/b" )

		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.ExactMatch | GafferScene.Filter.Result.DescendantMatch )
		self.assertEqual( m.match( "/a/b" ), GafferScene.Filter.Result.ExactMatch | GafferScene.Filter.Result.AncestorMatch )

		m.removePath( "/a" )
		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.DescendantMatch )
		self.assertEqual( m.match( "/a/b" ), GafferScene.Filter.Result.ExactMatch )

		m.removePath( "/a/b" )
		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.NoMatch )
		self.assertEqual( m.match( "/a/b" ), GafferScene.Filter.Result.NoMatch )

	def testRemovePathRemovesIntermediatePaths( self ) :

		m = GafferScene.PathMatcher()
		m.addPath( "/a/b/c" )

		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.DescendantMatch )
		self.assertEqual( m.match( "/a/b" ), GafferScene.Filter.Result.DescendantMatch )
		self.assertEqual( m.match( "/a/b/c" ), GafferScene.Filter.Result.ExactMatch )

		m.removePath( "/a/b/c" )

		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.NoMatch )
		self.assertEqual( m.match( "/a/b" ), GafferScene.Filter.Result.NoMatch )
		self.assertEqual( m.match( "/a/b/c" ), GafferScene.Filter.Result.NoMatch )

	def testRemoveEllipsis( self ) :

		m = GafferScene.PathMatcher()
		m.addPath( "/a/.../b" )

		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.DescendantMatch )
		self.assertEqual( m.match( "/a/c" ), GafferScene.Filter.Result.DescendantMatch )
		self.assertEqual( m.match( "/a/c/b" ), GafferScene.Filter.Result.ExactMatch | GafferScene.Filter.Result.DescendantMatch )

		m.removePath( "/a/.../b" )

		self.assertEqual( m.match( "/a" ), GafferScene.Filter.Result.NoMatch )
		self.assertEqual( m.match( "/a/c" ), GafferScene.Filter.Result.NoMatch )
		self.assertEqual( m.match( "/a/c/b" ), GafferScene.Filter.Result.NoMatch )

	def testAddPathReturnValue( self ) :

		m = GafferScene.PathMatcher()
		self.assertEqual( m.addPath( "/" ), True )
		self.assertEqual( m.addPath( "/a/b" ), True )
		self.assertEqual( m.addPath( "/a/b" ), False )
		self.assertEqual( m.addPath( "/a" ), True )
		self.assertEqual( m.addPath( "/" ), False )

		m = GafferScene.PathMatcher()
		self.assertEqual( m.addPath( "/a/b/c" ), True )
		self.assertEqual( m.addPath( "/a/b/c" ), False )
		self.assertEqual( m.addPath( "/" ), True )
		self.assertEqual( m.addPath( "/*" ), True )
		self.assertEqual( m.addPath( "/*" ), False )
		self.assertEqual( m.addPath( "/..." ), True )
		self.assertEqual( m.addPath( "/..." ), False )

		self.assertEqual( m.addPath( "/a/b/c/d" ), True )
		self.assertEqual( m.addPath( "/a/b/c/d" ), False )
		m.removePath( "/a/b/c/d" )
		self.assertEqual( m.addPath( "/a/b/c/d" ), True )
		self.assertEqual( m.addPath( "/a/b/c/d" ), False )

	def testRemovePathReturnValue( self ) :

		m = GafferScene.PathMatcher()

		self.assertEqual( m.removePath( "/" ), False )
		m.addPath( "/" )
		self.assertEqual( m.removePath( "/" ), True )
		self.assertEqual( m.removePath( "/" ), False )

		self.assertEqual( m.removePath( "/a/b/c" ), False )
		m.addPath( "/a/b/c" )
		self.assertEqual( m.removePath( "/a/b/c" ), True )
		self.assertEqual( m.removePath( "/a/b/c" ), False )

	def testEquality( self ) :

		m1 = GafferScene.PathMatcher()
		m2 = GafferScene.PathMatcher()

		self.assertEqual( m1, m2 )

		m1.addPath( "/a" )
		self.assertNotEqual( m1, m2 )

		m2.addPath( "/a" )
		self.assertEqual( m1, m2 )

		m2.addPath( "/a/b" )
		self.assertNotEqual( m1, m2 )

		m1.addPath( "/a/b" )
		self.assertEqual( m1, m2 )

		m1.addPath( "/a/b/.../c" )
		self.assertNotEqual( m1, m2 )

		m2.addPath( "/a/b/.../c" )
		self.assertEqual( m1, m2 )

		m2.addPath( "/c*" )
		self.assertNotEqual( m1, m2 )

		m1.addPath( "/c*" )
		self.assertEqual( m1, m2 )

	def testPaths( self ) :

		m = GafferScene.PathMatcher()
		self.assertEqual( m.paths(), [] )

		m.addPath( "/a/b" )
		self.assertEqual( m.paths(), [ "/a/b" ] )

		m.addPath( "/a/.../b" )
		self.assertEqual( m.paths(), [ "/a/b", "/a/.../b" ] )

		m.removePath( "/a/.../b" )
		self.assertEqual( m.paths(), [ "/a/b" ] )

		m.addPath( "/a/b/c*d*" )
		self.assertEqual( m.paths(), [ "/a/b", "/a/b/c*d*" ] )

		m.clear()
		self.assertEqual( m.paths(), [] )

		f = GafferScene.PathFilter()
		f["paths"].setValue(
			IECore.StringVectorData( [
				"/a",
				"/red*",
				"/green*Bloke*",
				"/somewhere/over/the/*",
				"/somewhere/over/the/*/skies/are/blue",
			] )
		)

	def testMultipleMatchTypes( self ) :

		m = GafferScene.PathMatcher( [
			"/a",
			"/a/b",
			"/a/b/c",
		] )

		r = GafferScene.Filter.Result

		for path, result in [
			( "/a", r.ExactMatch | r.DescendantMatch ),
			( "/a/b", r.ExactMatch | r.AncestorMatch | r.DescendantMatch ),
			( "/a/b/c", r.ExactMatch | r.AncestorMatch ),
			( "/a/b/d", r.AncestorMatch ),
		] :

			self.assertEqual( m.match( path ), int( result ) )

	def testAncestorMatch( self ) :

		m = GafferScene.PathMatcher( [
			"/a",
		] )

		r = GafferScene.Filter.Result

		for path, result in [
			( "/a/b", r.AncestorMatch ),
			( "/a/b/c", r.AncestorMatch ),
			( "/a/b/d", r.AncestorMatch ),
			( "/b/d", r.NoMatch ),
		] :

			self.assertEqual( m.match( path ), int( result ) )

	def testWildCardAncestorMatch( self ) :

		m = GafferScene.PathMatcher( [
			"/a*",
		] )

		r = GafferScene.Filter.Result

		for path, result in [
			( "/armadillo/brunches", r.AncestorMatch ),
			( "/a/b/c", r.AncestorMatch ),
			( "/a/b/d", r.AncestorMatch ),
			( "/b/d", r.NoMatch ),
			( "/armadillo", r.ExactMatch ),
		] :

			self.assertEqual( m.match( path ), int( result ) )

	def testRootAncestorMatch( self ) :

		m = GafferScene.PathMatcher( [
			"/",
		] )

		r = GafferScene.Filter.Result

		for path, result in [
			( "/armadillo/brunches", r.AncestorMatch ),
			( "/a/b/c", r.AncestorMatch ),
			( "/a/b/d", r.AncestorMatch ),
			( "/b/d", r.AncestorMatch ),
			( "/armadillo", r.AncestorMatch ),
		] :

			self.assertEqual( m.match( path ), int( result ) )

	def testInternedStringVectorData( self ) :

		paths = [
			IECore.InternedStringVectorData( [ "a", "b", "c" ] ),
			IECore.InternedStringVectorData( [ "w", "*" ] ),
			IECore.InternedStringVectorData( [ "e", "...", "f" ] ),
		]

		m = GafferScene.PathMatcher( paths )

		self.assertEqual( set( m.paths() ), set( [ "/a/b/c", "/w/*", "/e/.../f" ] ) )

		r = GafferScene.Filter.Result

		for path, result in [
			( "/a/b/c", r.ExactMatch ),
			( "/a/b", r.DescendantMatch ),
			( "/a/b/c/d", r.AncestorMatch ),
			( "/b/d", r.NoMatch ),
			( "/w/w", r.ExactMatch ),
			( "/e/f", r.ExactMatch | r.DescendantMatch ),
			( "/e/a/f", r.ExactMatch | r.DescendantMatch ),
		] :

			self.assertEqual(
				m.match( IECore.InternedStringVectorData( path[1:].split( "/" ) ) ),
				int( result ),
			)

		m.removePath( paths[0] )
		self.assertEqual( m.match( paths[0] ), r.NoMatch )

		m.addPath( paths[0] )
		self.assertEqual( m.match( paths[0] ), r.ExactMatch )

	def testInvalidPathArguments( self ) :

		self.assertRaises( TypeError, GafferScene.PathMatcher, [ None ] )

		m = GafferScene.PathMatcher()

		self.assertRaises( TypeError, m.match, None )

		self.assertRaises( TypeError, m.addPath, None )
		self.assertRaises( TypeError, m.removePath, None )

	def testPrune( self ) :

		m = GafferScene.PathMatcher( [
			"/a/b/c",
			"/a/.../c",
			"/a/b/c/d",
			"/a/b/...",
			"/a",
			"/c/d",
		] )

		self.assertEqual( m.prune( "/a/b" ), True )
		self.assertEqual( set( m.paths() ), set( [ "/a/.../c", "/a", "/c/d" ] ) )
		self.assertEqual( m.prune( "/a/b" ), False )

		self.assertEqual( m.prune( "/a" ), True )
		self.assertEqual( m.paths(), [ "/c/d" ] )
		self.assertEqual( m.prune( "/a" ), False )

		self.assertEqual( m.prune( "/c/d/e" ), False )
		self.assertEqual( m.paths(), [ "/c/d" ] )
		self.assertEqual( m.prune( "/c/d" ), True )
		self.assertEqual( m.paths(), [] )
		self.assertTrue( m.isEmpty() )
		self.assertEqual( m.prune( "/c/d" ), False )

	def testPruneRoot( self ) :

		m = GafferScene.PathMatcher( [
			"/a/b",
			"/a",
			"/.../c",
			"/...",
		] )

		self.assertEqual( m.prune( "/" ), True )
		self.assertEqual( m.paths(), [] )
		self.assertTrue( m.isEmpty() )

		# And again, this time with only
		# a single path, which also happens
		# to be the root.

		m = GafferScene.PathMatcher()
		m.addPath( "/" )
		self.assertEqual( m.paths(), [ "/" ] )

		self.assertTrue( m.prune( "/" ) )
		self.assertEqual( m.paths(), [] )
		self.assertTrue( m.isEmpty() )

	def testIsEmpty( self ) :

		m = GafferScene.PathMatcher( [] )
		self.assertTrue( m.isEmpty() )

		m.addPath( "/a" )
		self.assertFalse( m.isEmpty() )

		m.removePath( "/a" )
		self.assertTrue( m.isEmpty() )

		m.addPath( "/..." )
		self.assertFalse( m.isEmpty() )

		m.removePath( "/..." )
		self.assertTrue( m.isEmpty() )

		m.addPath( "/" )
		self.assertFalse( m.isEmpty() )

		m.removePath( "/" )
		self.assertTrue( m.isEmpty() )

	def testAddPaths( self ) :

		m1 = GafferScene.PathMatcher( [
			"/a",
			"/a/../b",
			"/b",
			"/b/c/d"
		] )

		m2 = GafferScene.PathMatcher( [
			"/a/b",
			"/a/../c",
			"/b/e",
			"/b/c/d/e/f",
			"/b/c/d/e/f/...",
			"/b/c/d/e/f/.../g",
		] )

		m = GafferScene.PathMatcher()
		self.assertEqual( m.addPaths( m1 ), True )
		self.assertEqual( m.paths(), m1.paths() )
		self.assertEqual( m.addPaths( m1 ), False )

		self.assertEqual( m.addPaths( m2 ), True )
		self.assertEqual( set( m.paths() ), set( m1.paths() + m2.paths() ) )
		self.assertEqual( m.addPaths( m2 ), False )

		m3 = GafferScene.PathMatcher( [
			"/b/e/..."
		] )

		self.assertEqual( m.addPaths( m3 ), True )
		self.assertEqual( set( m.paths() ), set( m1.paths() + m2.paths() + m3.paths() ) )
		self.assertEqual( m.addPaths( m3 ), False )

		m4 = GafferScene.PathMatcher( [
			"/b/e/f/g"
		] )

		self.assertEqual( m.addPaths( m4 ), True )
		self.assertEqual( set( m.paths() ), set( m1.paths() + m2.paths() + m3.paths() + m4.paths() ) )
		self.assertEqual( m.addPaths( m4 ), False )

	def testRemovePaths( self ) :

		m1 = GafferScene.PathMatcher( [
			"/a",
			"/a/../b",
			"/b",
			"/b/c/d"
		] )

		m2 = GafferScene.PathMatcher( [
			"/a/b",
			"/a/../c",
			"/b/e",
			"/b/c/d/e/f",
			"/b/c/d/e/f/...",
			"/b/c/d/e/f/.../g",
		] )

		m = GafferScene.PathMatcher()
		m.addPaths( m1 )
		m.addPaths( m2 )
		self.assertEqual( set( m.paths() ), set( m1.paths() + m2.paths() ) )
		self.assertFalse( m.isEmpty() )

		self.assertEqual( m.removePaths( m1 ), True )
		self.assertEqual( m.paths(), m2.paths() )
		self.assertEqual( m.removePaths( m1 ), False )
		self.assertFalse( m.isEmpty() )

		self.assertEqual( m.removePaths( m2 ), True )
		self.assertEqual( m.paths(), [] )
		self.assertEqual( m.removePaths( m2 ), False )
		self.assertTrue( m.isEmpty() )

	def testStrictWeakOrderingBug( self ) :

		m = GafferScene.PathMatcher( [
			"/c",
			"/*b"
		] )

		self.assertEqual( m.match( "/b"), GafferScene.Filter.Result.ExactMatch )

	def testPathsWithRootTerminator( self ) :

		m = GafferScene.PathMatcher()
		m.addPath( "/" )
		self.assertEqual( m.paths(), [ "/" ] )

	def testPathsWithRootTerminatorAndDescendant( self ) :

		m = GafferScene.PathMatcher()
		m.addPath( "/" )
		m.addPath( "/a/b/c" )
		self.assertEqual( m.paths(), [ "/", "/a/b/c" ] )

	def testEmptyPaths( self ) :

		m = GafferScene.PathMatcher()
		self.assertEqual( m.paths(), [] )

	def testRawIterator( self ) :

		GafferSceneTest.testPathMatcherRawIterator()

	def testIteratorPrune( self ) :

		GafferSceneTest.testPathMatcherIteratorPrune()

	def testCopyAndAddPaths( self ) :

		initialPaths = [
			"/a/b/c/d",
			"/a/b",
			"/e/f",
		]

		additionalPaths = [
			"/a/b/c/d/e",
			"/a/b/c/e",
			"/a/b/e",
			"/e",
			"/g"
		]

		m1 = GafferScene.PathMatcher( initialPaths )
		m2 = GafferScene.PathMatcher( m1 )

		self.assertEqual( set( m1.paths() ), set( initialPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

		for path in additionalPaths :
			m1.addPath( path )

		self.assertEqual( set( m1.paths() ), set( initialPaths + additionalPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

		# repeat, but add as PathMatcher rather than individual paths

		m1 = GafferScene.PathMatcher( initialPaths )
		m2 = GafferScene.PathMatcher( m1 )

		self.assertEqual( set( m1.paths() ), set( initialPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

		m1.addPaths( GafferScene.PathMatcher( additionalPaths ) )

		self.assertEqual( set( m1.paths() ), set( initialPaths + additionalPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

	def testCopyAndAddRoot( self ) :

		p1 = GafferScene.PathMatcher()
		p2 = GafferScene.PathMatcher( p1 )

		self.assertEqual( p1.paths(), [] )
		self.assertEqual( p2.paths(), [] )

		p2.addPath( "/" )
		self.assertEqual( p1.paths(), [] )
		self.assertEqual( p2.paths(), [ "/" ] )

	def testCopyAndRemovePath( self ) :

		initialPaths = [
			"/a/b/c/d/e",
			"/a/b",
			"/e/f",
			"/e/f/g",
			"/g"
		]

		pathsToRemove = [
			"/a/b",
			"/e/f/g",
			"/g",
		]

		m1 = GafferScene.PathMatcher( initialPaths )
		m2 = GafferScene.PathMatcher( m1 )

		self.assertEqual( set( m1.paths() ), set( initialPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

		for path in pathsToRemove :
			m1.removePath( path )

		self.assertEqual( set( m1.paths() ), set( initialPaths ) - set( pathsToRemove ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

		# repeat, but add as PathMatcher rather than individual paths

		m1 = GafferScene.PathMatcher( initialPaths )
		m2 = GafferScene.PathMatcher( m1 )

		self.assertEqual( set( m1.paths() ), set( initialPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

		m1.removePaths( GafferScene.PathMatcher( pathsToRemove ) )

		self.assertEqual( set( m1.paths() ), set( initialPaths ) - set( pathsToRemove ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

	def testCopyAndPrunePath( self ) :

		initialPaths = [
			"/a/b/c/d/e",
			"/a/b",
			"/e/f",
			"/e/f/g",
			"/g"
		]

		pathsToPrune = [
			"/a/b",
			"/e/f/g",
			"/g",
		]

		expectedPaths = [
			"/e/f",
		]

		m1 = GafferScene.PathMatcher( initialPaths )
		m2 = GafferScene.PathMatcher( m1 )

		self.assertEqual( set( m1.paths() ), set( initialPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

		for path in pathsToPrune :
			m1.prune( path )

		self.assertEqual( set( m1.paths() ), set( expectedPaths ) )
		self.assertEqual( set( m2.paths() ), set( initialPaths ) )

	def testFind( self ) :

		GafferSceneTest.testPathMatcherFind()

	def testSubTree( self ) :

		paths = [
			"/a/b/c/d/e",
			"/d/b/c/d",
			"/a",
			"/a/b/c",
		]

		m1 = GafferScene.PathMatcher( paths )
		self.assertEqual( set( m1.paths() ), set( paths ) )

		expectedPaths = [
			"/d/e",
			"/"
		]

		m2 = m1.subTree( "/a/b/c" )
		self.assertEqual( set( m1.paths() ), set( paths ) )
		self.assertEqual( set( m2.paths() ), set( expectedPaths ) )

		m1.addPath( "/a/b/c/d/f" )
		self.assertEqual( set( m1.paths() ), set( paths + [ "/a/b/c/d/f" ] ) )
		self.assertEqual( set( m2.paths() ), set( expectedPaths ) )

		m2.addPath( "/d/e/g" )
		self.assertEqual( set( m1.paths() ), set( paths + [ "/a/b/c/d/f" ] ) )
		self.assertEqual( set( m2.paths() ), set( expectedPaths + [ "/d/e/g" ] ) )

	def testNonExistentSubtree( self ) :

		m1 = GafferScene.PathMatcher( "/a/b" )
		m2 = m1.subTree( "/wot?" )

		self.assertEqual( m2.paths(), [] )

	def testSubTreeWithNonTerminatingRoot( self ) :

		m1 = GafferScene.PathMatcher( [ "/a/b/c/d" ] )
		m2 = m1.subTree( "/a" )
		self.assertEqual( m2.paths(), [ "/b/c/d" ] )

	def testAddPathsWithPrefix( self ) :

		paths = [
			"/a/b",
			"/e/d",
			"/",
		]

		prefixedPaths = [
			"/x/y/z/a/b",
			"/x/y/z/e/d",
			"/x/y/z",
		]

		m1 = GafferScene.PathMatcher( paths )
		self.assertEqual( set( m1.paths() ), set( paths ) )

		m2 = GafferScene.PathMatcher()
		self.assertTrue( m2.addPaths( m1, "/x/y/z" ) )
		self.assertFalse( m2.addPaths( m1, "/x/y/z" ) )

		self.assertEqual( set( m1.paths() ), set( paths ) )
		self.assertEqual( set( m2.paths() ), set( prefixedPaths ) )

		self.assertTrue( m1.addPath( "/b/c" ) )
		self.assertEqual( set( m1.paths() ), set( paths + [ "/b/c" ] ) )
		self.assertEqual( set( m2.paths() ), set( prefixedPaths ) )

	def testAddEmptyPathsWithPrefix( self ) :

		m = GafferScene.PathMatcher()
		m.addPaths( GafferScene.PathMatcher(), "/x/y/z" )
		self.assertEqual( m.paths(), [] )
		self.assertTrue( m.isEmpty() )

	def testPrefixNotAdded( self ) :

		m = GafferScene.PathMatcher()
		m.addPaths( GafferScene.PathMatcher( [ "/a" ] ), "/prefix" )
		self.assertEqual( m.paths(), [ "/prefix/a" ] )

	def testEmptyStringIsNotAPath( self ) :

		m = GafferScene.PathMatcher()
		self.assertFalse( m.addPath( "" ) )
		self.assertTrue( m.isEmpty() )
		self.assertEqual( m.paths(), [] )

		m.addPath( "/" )
		self.assertEqual( m.paths(), [ "/" ] )
		self.assertFalse( m.removePath( "" ) )
		self.assertFalse( m.isEmpty() )
		self.assertEqual( m.paths(), [ "/" ] )

		m.addPath( "/a" )
		self.assertEqual( m.paths(), [ "/", "/a" ] )
		self.assertFalse( m.prune( "" ) )
		self.assertEqual( m.paths(), [ "/", "/a" ] )

		self.assertEqual( m.match( "" ), GafferScene.Filter.Result.NoMatch )

		s = m.subTree( "" )
		self.assertTrue( s.isEmpty() )

	def testRepr( self ) :

		m1 = GafferScene.PathMatcher()
		m2 = GafferScene.PathMatcher( [
			"/a/b",
			"/a/*"
		] )

		m1c = eval( repr( m1 ) )
		m2c = eval( repr( m2 ) )

		self.assertEqual( m1, m1c )
		self.assertEqual( m2, m2c )

if __name__ == "__main__":
	unittest.main()
