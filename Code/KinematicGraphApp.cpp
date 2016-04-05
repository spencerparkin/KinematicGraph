// KinematicGraphApp.cpp

#include "KinematicGraphApp.h"
#include "KinematicGraphFrame.h"
#include "KinematicGraph.h"

wxIMPLEMENT_APP( KinematicGraphApp );

KinematicGraphApp::KinematicGraphApp( void )
{
	kinematicGraph = new KinematicGraph();
}

/*virtual*/ KinematicGraphApp::~KinematicGraphApp( void )
{
	delete kinematicGraph;
}

/*virtual*/ bool KinematicGraphApp::OnInit( void )
{
	if( !wxApp::OnInit() )
		return false;

	int idA = kinematicGraph->InsertVertex( c3ga::vectorE3GA( c3ga::vectorE3GA::coord_e1_e2_e3, 0.f, 0.f, 0.f ) );
	int idB = kinematicGraph->InsertVertex( c3ga::vectorE3GA( c3ga::vectorE3GA::coord_e1_e2_e3, 1.f, 0.f, 0.f ) );
	int idC = kinematicGraph->InsertVertex( c3ga::vectorE3GA( c3ga::vectorE3GA::coord_e1_e2_e3, 1.f, 1.f, 0.f ) );
	int idD = kinematicGraph->InsertVertex( c3ga::vectorE3GA( c3ga::vectorE3GA::coord_e1_e2_e3, 2.f, 1.f, 0.f ) );

	kinematicGraph->ConnectVertices( idA, idB );
	kinematicGraph->ConnectVertices( idB, idC );
	kinematicGraph->ConnectVertices( idC, idD );

	KinematicGraphFrame* frame = new KinematicGraphFrame( nullptr, wxDefaultPosition, wxSize( 700, 700 ) );
	frame->Show( true );

	return true;
}

/*virtual*/ int KinematicGraphApp::OnExit( void )
{
	return 0;
}

// KinematicGraphApp.cpp