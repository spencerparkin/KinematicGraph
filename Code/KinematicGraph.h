// KinematicGraph.h

#pragma once

#include "C3GA/c3ga.h"
#include <wx/glcanvas.h>
#include <list>
#include <map>

class KinematicGraph
{
	friend class Element;

public:

	KinematicGraph( void );
	~KinematicGraph( void );

	int InsertVertex( const c3ga::vectorE3GA& location );
	bool RemoveVertex( int id );
	bool ConnectVertices( int idA, int idB );
	bool DisconnectVertices( int idA, int idB );
	void Clear( void );

	void Render( GLenum renderMode );
	
	void SetSelectedId( int id ) { selectedId = id; }
	int GetSelectedId( void ) { return selectedId; }

	void MoveVertex( int id, const c3ga::vectorE3GA& delta );
	bool GetVertexLocation( int id, c3ga::vectorE3GA& location );
	bool SetVertexStationary( int id, bool stationary );
	bool GetVertexStationary( int id );

private:

	float epsilon;

	class Element;
	class Edge;
	class Vertex;

	typedef std::list< Element* > ElementList;
	typedef std::list< Edge* > EdgeList;
	typedef std::list< Vertex* > VertexList;
	typedef std::map< int, Element* > ElementMap;

	class Element
	{
	public:
		int id;
		KinematicGraph* kinematicGraph;
		c3ga::vectorE3GA color;
		Element( int id, KinematicGraph* kinematicGraph );
		virtual ~Element( void );
		virtual int ReturnType( void ) const = 0;
		virtual void Render( GLenum renderMode );
	};

	class Edge : public Element
	{
	public:
		float length;
		Vertex* vertex[2];
		Edge( int id, KinematicGraph* kinematicGraph );
		virtual ~Edge( void );
		static int Type( void ) { return 0; }
		virtual int ReturnType( void ) const override { return Type(); }
		virtual void Render( GLenum renderMode ) override;
		Vertex* Follow( Vertex* vertex );
		float CalcLength( void );
	};

	class Vertex : public Element
	{
	public:
		c3ga::vectorE3GA location;
		c3ga::vectorE3GA station;
		bool stationary;
		EdgeList edgeList;
		int key;
		Vertex( int id, KinematicGraph* kinematicGraph );
		virtual ~Vertex( void );
		static int Type( void ) { return 1; }
		virtual int ReturnType( void ) const override { return Type(); }
		virtual void Render( GLenum renderMode ) override;
		Edge* Follow( Vertex* vertex, EdgeList::iterator* foundIter = nullptr );
	};

	struct Move
	{
		Vertex* vertex;
		c3ga::vectorE3GA delta;
	};

	typedef std::list< Move > MoveList;

	void MoveVertexUnconstrained( Vertex* vertex, const c3ga::vectorE3GA& delta );
	bool FoundOnMoveList( const MoveList& moveList, Vertex* vertex );

	int newId;
	int selectedId;
	int key;

	ElementMap elementMap;

	template< typename ElementType > ElementType* FindElement( int id, ElementMap::iterator* foundIter = nullptr );
};

// KinematicGraph.h
