// KinematicGraphFrame.h

#pragma once

#include <wx/frame.h>

class KinematicGraphCanvas;

class KinematicGraphFrame : public wxFrame
{
public:

	KinematicGraphFrame( wxWindow* parent, const wxPoint& pos, const wxSize& size );
	virtual ~KinematicGraphFrame( void );

private:

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_Clear,
		ID_About,
	};

	void OnExit( wxCommandEvent& event );
	void OnClear( wxCommandEvent& event );
	void OnAbout( wxCommandEvent& event );

	KinematicGraphCanvas* canvas;
};

// KinematicGraphFrame.h