// KinematicGraphFrame.cpp

#include "KinematicGraphFrame.h"
#include "KinematicGraphCanvas.h"
#include "KinematicGraph.h"
#include "KinematicGraphApp.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/aboutdlg.h>

KinematicGraphFrame::KinematicGraphFrame( wxWindow* parent, const wxPoint& pos, const wxSize& size ) : wxFrame( parent, wxID_ANY, "Kinematic Graph", pos, size )
{
	wxMenu* programMenu = new wxMenu();
	wxMenuItem* clearMenuItem = new wxMenuItem( programMenu, ID_Clear, "Clear", "Clear the graph." );
	wxMenuItem* exitMenuItem = new wxMenuItem( programMenu, ID_Exit, "Exit", "Exit this program." );
	programMenu->Append( clearMenuItem );
	programMenu->AppendSeparator();
	programMenu->Append( exitMenuItem );

	wxMenu* helpMenu = new wxMenu();
	wxMenuItem* aboutMenuItem = new wxMenuItem( helpMenu, ID_About, "About", "Show about-box." );
	helpMenu->Append( aboutMenuItem );

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append( programMenu, "Program" );
	menuBar->Append( helpMenu, "Help" );
	SetMenuBar( menuBar );

	canvas = new KinematicGraphCanvas( this );

	wxBoxSizer* boxSizer = new wxBoxSizer( wxVERTICAL );
	boxSizer->Add( canvas, 1, wxALL | wxGROW, 0 );
	SetSizer( boxSizer );

	Bind( wxEVT_MENU, &KinematicGraphFrame::OnClear, this, ID_Clear );
	Bind( wxEVT_MENU, &KinematicGraphFrame::OnExit, this, ID_Exit );
	Bind( wxEVT_MENU, &KinematicGraphFrame::OnAbout, this, ID_About );
}

/*virtual*/ KinematicGraphFrame::~KinematicGraphFrame( void )
{
}

void KinematicGraphFrame::OnExit( wxCommandEvent& event )
{
	Close( true );
}

void KinematicGraphFrame::OnClear( wxCommandEvent& event )
{
	KinematicGraph* kinematicGraph = wxGetApp().GetKinematicGraph();
	kinematicGraph->Clear();
	Refresh();
}

void KinematicGraphFrame::OnAbout( wxCommandEvent& event )
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName( "Kinenatic Graph" );
	aboutDialogInfo.SetVersion( "0.1" );
	aboutDialogInfo.SetDescription( "Play around with a generalization of the IK problem." );
	aboutDialogInfo.SetCopyright( "Copyright (C) 2016 -- Spencer T. Parkin <SpencerTParkin@gmail.com>" );

	wxAboutBox( aboutDialogInfo );
}

// KinematicGraphFrame.cpp