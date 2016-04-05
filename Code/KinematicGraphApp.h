// KinematicGraphApp.h

#pragma once

#include <wx/setup.h>
#include <wx/app.h>

class KinematicGraph;

class KinematicGraphApp : public wxApp
{
public:

	KinematicGraphApp( void );
	virtual ~KinematicGraphApp( void );

	virtual bool OnInit( void ) override;
	virtual int OnExit( void ) override;

	KinematicGraph* GetKinematicGraph( void ) { return kinematicGraph; }

private:

	KinematicGraph* kinematicGraph;
};

wxDECLARE_APP( KinematicGraphApp );

// KinematicGraphApp.h