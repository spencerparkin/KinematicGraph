// KinematicGraph.cpp

#include "KinematicGraph.h"

KinematicGraph::KinematicGraph( void )
{
	newId = 1;
	selectedId = 0;
	key = 0;
	epsilon = 1e-5f;
}

KinematicGraph::~KinematicGraph( void )
{
	Clear();
}

void KinematicGraph::Clear( void )
{
	while( elementMap.size() > 0 )
	{
		ElementMap::iterator elementIter = elementMap.begin();
		Element* element = elementIter->second;
		delete element;
		elementMap.erase( elementIter );
	}
}

bool KinematicGraph::GetVertexLocation( int id, c3ga::vectorE3GA& location )
{
	Vertex* vertex = FindElement< Vertex >( id );
	if( !vertex )
		return false;

	location = vertex->location;
	return true;
}

void KinematicGraph::Render( GLenum renderMode )
{
	// Draw all edges first.
	glLineWidth( 2.f );
	ElementMap::iterator elementIter = elementMap.begin();
	while( elementIter != elementMap.end() )
	{
		Element* element = elementIter->second;
		if( element->ReturnType() == Edge::Type() )
			element->Render( renderMode );
		elementIter++;
	}

	// Draw all vertices last.
	glPointSize( 10.f );
	elementIter = elementMap.begin();
	while( elementIter != elementMap.end() )
	{
		Element* element = elementIter->second;
		if( element->ReturnType() == Vertex::Type() )
			element->Render( renderMode );
		elementIter++;
	}
}

template< typename ElementType >
ElementType* KinematicGraph::FindElement( int id, ElementMap::iterator* foundIter /*= nullptr*/ )
{
	ElementMap::iterator elementIter = elementMap.find( id );
	if( elementIter == elementMap.end() )
		return nullptr;

	Element* element = elementIter->second;
	if( element->ReturnType() != ElementType::Type() )
		return nullptr;

	if( foundIter )
		*foundIter = elementIter;

	return( ( ElementType* )element );
}

int KinematicGraph::InsertVertex( const c3ga::vectorE3GA& location )
{
	Vertex* vertex = new Vertex( newId++, this );
	vertex->location = location;
	elementMap.insert( std::pair< int, Element* >( vertex->id, vertex ) );
	return vertex->id;
}

bool KinematicGraph::RemoveVertex( int id )
{
	ElementMap::iterator elementIter;
	Vertex* vertex = FindElement< Vertex >( id, &elementIter );
	if( !vertex )
		return false;

	while( vertex->edgeList.size() > 0 )
	{
		EdgeList::iterator edgeIter = vertex->edgeList.begin();
		Edge* edge = *edgeIter;
		if( !DisconnectVertices( vertex->id, edge->Follow( vertex )->id ) )
			return false;
	}

	elementMap.erase( elementIter );
	delete vertex;
	return true;
}

bool KinematicGraph::ConnectVertices( int idA, int idB )
{
	if( idA == idB )
		return false;

	Vertex* vertexA = FindElement< Vertex >( idA );
	if( !vertexA )
		return false;

	Vertex* vertexB = FindElement< Vertex >( idB );
	if( !vertexB )
		return false;

	Edge* edgeA = vertexA->Follow( vertexB );
	if( edgeA )
		return false;

	Edge* edgeB = vertexB->Follow( vertexA );
	if( edgeB )
		return false;

	Edge* edge = new Edge( newId++, this );
	edge->vertex[0] = vertexA;
	edge->vertex[1] = vertexB;
	edge->length = edge->CalcLength();
	elementMap.insert( std::pair< int, Element* >( edge->id, edge ) );

	vertexA->edgeList.push_back( edge );
	vertexB->edgeList.push_back( edge );

	return true;
}

bool KinematicGraph::DisconnectVertices( int idA, int idB )
{
	Vertex* vertexA = FindElement< Vertex >( idA );
	if( !vertexA )
		return false;

	Vertex* vertexB = FindElement< Vertex >( idB );
	if( !vertexB )
		return false;

	EdgeList::iterator edgeIterA;
	Edge* edgeA = vertexA->Follow( vertexB, &edgeIterA );
	if( !edgeA )
		return false;

	EdgeList::iterator edgeIterB;
	Edge* edgeB = vertexB->Follow( vertexA, &edgeIterB );
	if( !edgeB )
		return false;

	if( edgeA != edgeB )
		return false;

	vertexA->edgeList.erase( edgeIterA );
	vertexB->edgeList.erase( edgeIterB );

	ElementMap::iterator elementIter;
	Edge* edge = FindElement< Edge >( edgeA->id, &elementIter );
	if( !edge )
		return false;

	elementMap.erase( elementIter );
	delete edge;
	return true;
}

bool KinematicGraph::SetVertexStationary( int id, bool stationary )
{
	Vertex* vertex = FindElement< Vertex >( id );
	if( !vertex )
		return false;

	vertex->stationary = stationary;
	if( stationary )
		vertex->station = vertex->location;
	else
		vertex->station.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.f, 0.f, 0.f );

	return true;
}

bool KinematicGraph::GetVertexStationary( int id )
{
	Vertex* vertex = FindElement< Vertex >( id );
	if( !vertex )
		return false;

	return vertex->stationary;
}

void KinematicGraph::MoveVertex( int id, const c3ga::vectorE3GA& delta )
{
	Vertex* vertex = FindElement< Vertex >( id );
	if( !vertex )
		return;

	Move move;
	move.vertex = vertex;
	move.delta = delta;

	MoveList moveQueue;
	moveQueue.push_back( move );

	while( moveQueue.size() > 0 )
	{
		// Flush the queue.
		while( moveQueue.size() > 0 )
		{
			MoveList::iterator moveIter = moveQueue.begin();
			move = *moveIter;
			moveQueue.erase( moveIter );
			key++;
			MoveVertexUnconstrained( move.vertex, move.delta );
		}

		// Now go obey the constraints.
		ElementMap::iterator elementIter = elementMap.begin();
		while( elementIter != elementMap.end() )
		{
			Element* element = elementIter->second;
			if( element->ReturnType() == Vertex::Type() )
			{
				Vertex* vertex = ( Vertex* )element;
				if( vertex->stationary )
				{
					float error = c3ga::norm( vertex->station - vertex->location );
					if( error > epsilon )
					{
						c3ga::vectorE3GA deltaDir = c3ga::unit( vertex->station - vertex->location );

						move.vertex = vertex;
						move.delta = deltaDir * ( error * 0.5f );
						moveQueue.push_back( move );

						break;
					}
				}
			}
			else if( element->ReturnType() == Edge::Type() )
			{
				Edge* edge = ( Edge* )element;
				float error = edge->CalcLength() - edge->length;
				if( fabs( error ) > epsilon )
				{
					c3ga::vectorE3GA deltaDir = c3ga::unit( edge->vertex[1]->location - edge->vertex[0]->location );

					move.vertex = edge->vertex[0];
					move.delta = deltaDir * ( error * 0.5f );
					moveQueue.push_back( move );

					//move.vertex = edge->vertex[1];
					//move.delta = deltaDir * ( -error * 0.5f );
					//moveQueue.push_back( move );

					break;
				}
			}

			elementIter++;
		}
	}
}

// TODO: Note that I have seen this routine lock-up (i.e., loop forever);
//       so there are some cases where it's possible.
void KinematicGraph::MoveVertexUnconstrained( Vertex* vertex, const c3ga::vectorE3GA& delta )
{
	Move move;
	move.vertex = vertex;
	move.delta = delta;

	MoveList moveQueue;
	moveQueue.push_back( move );

	while( moveQueue.size() > 0 )
	{
		MoveList::iterator moveIter = moveQueue.begin();
		move = *moveIter;
		moveQueue.erase( moveIter );

		Vertex* vertex = move.vertex;

		wxASSERT( vertex->key != key );
		vertex->key = key;

		vertex->location = vertex->location + move.delta;

		EdgeList::iterator edgeIter = vertex->edgeList.begin();
		while( edgeIter != vertex->edgeList.end() )
		{
			Edge* edge = *edgeIter;
			
			Vertex* otherVertex = edge->Follow( vertex );
			if( otherVertex->key != key && !FoundOnMoveList( moveQueue, otherVertex ) )
			{
				c3ga::vectorE3GA deltaDir = c3ga::unit( otherVertex->location - vertex->location );
				float currentLength = edge->CalcLength();
				float error = edge->length - currentLength;
				if( fabs( error ) > epsilon )
				{
					move.vertex = otherVertex;
					move.delta = deltaDir * error;
					moveQueue.push_back( move );
				}
			}

			edgeIter++;
		}
	}
}

bool KinematicGraph::FoundOnMoveList( const MoveList& moveList, Vertex* vertex )
{
	MoveList::const_iterator moveIter = moveList.begin();
	while( moveIter != moveList.end() )
	{
		const Move& move = *moveIter;
		if( move.vertex == vertex )
			return true;
		moveIter++;
	}
	return false;
}

KinematicGraph::Element::Element( int id, KinematicGraph* kinematicGraph )
{
	this->id = id;
	this->kinematicGraph = kinematicGraph;

	color.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.f, 0.f, 0.f );
}

/*virtual*/ KinematicGraph::Element::~Element( void )
{
}

/*virtual*/ void KinematicGraph::Element::Render( GLenum renderMode )
{
	if( renderMode == GL_SELECT )
		glLoadName( id );

	if( id == kinematicGraph->selectedId )
		glColor3f( 1.f, 0.f, 0.f );
	else
		glColor3f( color.get_e1(), color.get_e2(), color.get_e3() );
}

KinematicGraph::Edge::Edge( int id, KinematicGraph* kinematicGraph ) : Element( id, kinematicGraph )
{
	length = 0.f;
	vertex[0] = nullptr;
	vertex[1] = nullptr;
}

/*virtual*/ KinematicGraph::Edge::~Edge( void )
{
}

/*virtual*/ void KinematicGraph::Edge::Render( GLenum renderMode )
{
	Element::Render( renderMode );

	if( vertex[0] && vertex[1] )
	{
		c3ga::vectorE3GA* locationA = &vertex[0]->location;
		c3ga::vectorE3GA* locationB = &vertex[1]->location;

		if( renderMode == GL_RENDER )
		{
			glBegin( GL_LINES );
			glVertex3f( locationA->get_e1(), locationA->get_e2(), locationA->get_e3() );
			glVertex3f( locationB->get_e1(), locationB->get_e2(), locationB->get_e3() );
			glEnd();
		}
		else if( renderMode == GL_SELECT )
		{
			c3ga::vectorE3GA pointA = *locationA + ( *locationB - *locationA ) * ( 1.f / 3.f );
			c3ga::vectorE3GA pointB = *locationA + ( *locationB - *locationA ) * ( 2.f / 3.f );

			glBegin( GL_LINES );
			glVertex3f( pointA.get_e1(), pointA.get_e2(), pointA.get_e3() );
			glVertex3f( pointB.get_e1(), pointB.get_e2(), pointB.get_e3() );
			glEnd();
		}
	}
}

KinematicGraph::Vertex* KinematicGraph::Edge::Follow( Vertex* vertex )
{
	if( this->vertex[0] == vertex )
		return this->vertex[1];
	if( this->vertex[1] == vertex )
		return this->vertex[0];
	return nullptr;
}

float KinematicGraph::Edge::CalcLength( void )
{
	return c3ga::norm( vertex[0]->location - vertex[1]->location );
}

KinematicGraph::Vertex::Vertex( int id, KinematicGraph* kinematicGraph ) : Element( id, kinematicGraph )
{
	stationary = false;
	
	location.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.f, 0.f, 0.f );
	station.set( c3ga::vectorE3GA::coord_e1_e2_e3, 0.f, 0.f, 0.f );

	key = 0;
}

/*virtual*/ KinematicGraph::Vertex::~Vertex( void )
{
}

/*virtual*/ void KinematicGraph::Vertex::Render( GLenum renderMode )
{
	Element::Render( renderMode );

	glBegin( GL_POINTS );
	glVertex3f( location.get_e1(), location.get_e2(), location.get_e3() );
	glEnd();

	if( renderMode == GL_RENDER && stationary )
	{
		glLineWidth( 1.5f );
		glColor3f( 1.f, 0.f, 0.f );
		glBegin( GL_LINE_LOOP );
		int segments = 16;
		float radius = 0.15f;
		for( int i = 0; i < segments; i++ )
		{
			float angle = float(i) / float( segments ) * 2.f * M_PI;
			glVertex2f( station.get_e1() + radius * cos( angle ), station.get_e2() + radius * sin( angle ) );
		}
		glEnd();
	}
}

KinematicGraph::Edge* KinematicGraph::Vertex::Follow( Vertex* vertex, EdgeList::iterator* foundIter /*= nullptr*/ )
{
	for( EdgeList::iterator edgeIter = edgeList.begin(); edgeIter != edgeList.end(); edgeIter++ )
	{
		Edge* edge = *edgeIter;
		if( edge->Follow( this ) == vertex )
		{
			if( foundIter )
				*foundIter = edgeIter;

			return edge;
		}
	}

	return nullptr;
}

// KinematicGraph.cpp