//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2011, John Haddon. All rights reserved.
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

#include "Gaffer/Action.h"

#include "Gaffer/BackgroundTask.h"
#include "Gaffer/ScriptNode.h"

#include "IECore/Exception.h"
#include "IECore/RunTimeTyped.h"

using namespace Gaffer;

//////////////////////////////////////////////////////////////////////////
// Action implementation
//////////////////////////////////////////////////////////////////////////

IE_CORE_DEFINERUNTIMETYPED( Action );

Action::Action( bool cancelBackgroundTasks )
	:	m_done( false ), m_cancelBackgroundTasks( cancelBackgroundTasks )
{
}

Action::~Action()
{
}

void Action::enact( ActionPtr action )
{
	ScriptNode *s = IECore::runTimeCast<ScriptNode>( action->subject() );
	if( !s )
	{
		s = action->subject()->ancestor<ScriptNode>();
	}

	if( s )
	{
		s->addAction( action );
	}
	else
	{
		action->doAction();
	}

}

void Action::doAction()
{
	if( m_done )
	{
		throw IECore::Exception( "Action cannot be done again without being undone first." );
	}
	if( m_cancelBackgroundTasks )
	{
		BackgroundTask::cancelAffectedTasks( subject() );
	}
	m_done = true;
}

void Action::undoAction()
{
	if( !m_done )
	{
		throw IECore::Exception( "Action cannot be undone without being done first." );
	}
	if( m_cancelBackgroundTasks )
	{
		BackgroundTask::cancelAffectedTasks( subject() );
	}
	m_done = false;
}

bool Action::canMerge( const Action *other ) const
{
	return true;
}

void Action::merge( const Action *other )
{
}

//////////////////////////////////////////////////////////////////////////
// SimpleAction implementation and Action::enact() convenience overload.
//////////////////////////////////////////////////////////////////////////

namespace Gaffer
{

class SimpleAction : public Action
{

	public :

		SimpleAction( const GraphComponentPtr subject, const Function &doFn, const Function &undoFn, bool cancelBackgroundTasks )
			:	Action( cancelBackgroundTasks ), m_subject( subject.get() ), m_doFn( doFn ), m_undoFn( undoFn )
		{
			// In the documentation for Action::enact(), we promise that we'll keep
			// the subject alive for as long as the Functions are in use. If the subject
			// is a ScriptNode, that is taken care of for us, as the ScriptNode has ownership
			// of the SimpleAction. If the subject is not a ScriptNode, we must increment
			// the reference count and decrement it again in our destructor.
			if( !m_subject->isInstanceOf( Gaffer::ScriptNode::staticTypeId() ) )
			{
				m_subject->addRef();
			}
		}

		~SimpleAction() override
		{
			if( !m_subject->isInstanceOf( Gaffer::ScriptNode::staticTypeId() ) )
			{
				m_subject->removeRef();
			}
		}

		IE_CORE_DECLARERUNTIMETYPEDEXTENSION( Gaffer::SimpleAction, SimpleActionTypeId, Action );

	protected :

		GraphComponent *subject() const override
		{
			return m_subject;
		}

		void doAction() override
		{
			Action::doAction();
			if( m_doFn )
			{
				m_doFn();
			}
		}

		void undoAction() override
		{
			Action::undoAction();
			if( m_undoFn )
			{
				m_undoFn();
			}
		}

		bool canMerge( const Action *other ) const override
		{
			return false;
		}

		void merge( const Action *other ) override
		{
		}

	private :

		GraphComponent *m_subject;
		Function m_doFn;
		Function m_undoFn;

};

IE_CORE_DEFINERUNTIMETYPED( SimpleAction );

void Action::enact( GraphComponentPtr subject, const Function &doFn, const Function &undoFn, bool cancelBackgroundTasks )
{
	/// \todo We might want to optimise away the construction of a SimpleAction
	/// when we know that enact() will just call doFn and throw it away (when undo
	/// is disabled). If we do that we should make it easy for other subclasses to do
	/// the same.
	enact( new SimpleAction( subject, doFn, undoFn, cancelBackgroundTasks ) );
}

} // namespace Gaffer
