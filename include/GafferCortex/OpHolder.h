//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011-2012, John Haddon. All rights reserved.
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

#ifndef GAFFERCORTEX_OPHOLDER_H
#define GAFFERCORTEX_OPHOLDER_H

#include "GafferCortex/Export.h"
#include "GafferCortex/ParameterisedHolder.h"

namespace IECore
{

IE_CORE_FORWARDDECLARE( Op )

} // namespace IECore

namespace GafferCortex
{

IE_CORE_FORWARDDECLARE( ParameterHandler )

class GAFFERCORTEX_API OpHolder : public ParameterisedHolderComputeNode
{

	public :

		GAFFER_NODE_DECLARE_TYPE( GafferCortex::OpHolder, OpHolderTypeId, ParameterisedHolderComputeNode );

		OpHolder( const std::string &name=defaultName<OpHolder>() );

		void setParameterised( IECore::RunTimeTypedPtr parameterised, bool keepExistingValues=false ) override;

		/// Convenience function which calls setParameterised( className, classVersion, "IECORE_OP_PATHS", keepExistingValues )
		void setOp( const std::string &className, int classVersion, bool keepExistingValues=false );
		/// Convenience function which returns runTimeCast<Op>( getParameterised() );
		IECore::Op *getOp( std::string *className = nullptr, int *classVersion = nullptr );
		const IECore::Op *getOp( std::string *className = nullptr, int *classVersion = nullptr ) const;

		void affects( const Gaffer::Plug *input, AffectedPlugsContainer &outputs ) const override;

	protected :

		void hash( const Gaffer::ValuePlug *output, const Gaffer::Context *context, IECore::MurmurHash &h ) const override;
		void compute( Gaffer::ValuePlug *output, const Gaffer::Context *context ) const override;

	private :

		ParameterHandlerPtr m_resultParameterHandler;

};

IE_CORE_DECLAREPTR( OpHolder )

} // namespace GafferCortex

#endif // GAFFERCORTEX_OPHOLDER_H
